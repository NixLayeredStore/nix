#include "local-store.hh"

namespace nix {

/**
 * Configuration for `LocalOverlayStore`.
 */
struct LocalOverlayStoreConfig : virtual LocalStoreConfig
{
    // FIXME why doesn't this work?
    // using LocalStoreConfig::LocalStoreConfig;

    LocalOverlayStoreConfig(const StringMap & params)
        : StoreConfig(params)
        , LocalFSStoreConfig(params)
        , LocalStoreConfig(params)
    { }

    const Setting<std::string> lowerStoreUri{(StoreConfig*) this, "", "lower-store",
        R"(
          [Store URL](@docroot@/command-ref/new-cli/nix3-help-stores.md#store-url-format)
          for the lower store. The default is `auto` (i.e. use the Nix daemon or `/nix/store` directly).

          Must be a store with a store dir on the file system.
          Must be used as OverlayFS lower layer for this store's store dir.
        )"};

    const Setting<std::string> upperLayer{(StoreConfig*) this, "", "upper-layer",
        R"(
          Must be used as OverlayFS upper layer for this store's store dir.
        )"};

    Setting<bool> checkMount{(StoreConfig*) this, true, "check-mount",
        "Check that the overlay filesystem is correctly mounted."};

    const std::string name() override { return "Experimental Local Overlay Store"; }

    std::string doc() override
    {
        return
          ""
          // FIXME write docs
          //#include "local-overlay-store.md"
          ;
    }

    /**
     * Given a store path, get its location (if it is exists) in the
     * upper layer of the overlayfs.
     */
    Path toUpperPath(const StorePath & path);
};

/**
 * Variation of local store using overlayfs for the store dir.
 */
class LocalOverlayStore : public virtual LocalOverlayStoreConfig, public virtual LocalStore
{
    /**
     * The store beneath us.
     *
     * Our store dir should be an overlay fs where the lower layer
     * is that store's store dir, and the upper layer is some
     * scratch storage just for us.
     */
    ref<LocalFSStore> lowerStore;

public:
    LocalOverlayStore(const Params & params);

    LocalOverlayStore(std::string scheme, std::string path, const Params & params)
        : LocalOverlayStore(params)
    {
        throw UnimplementedError("LocalOverlayStore");
    }

    static std::set<std::string> uriSchemes()
    { return {}; }

    std::string getUri() override
    {
        return "local-overlay";
    }

private:
    // Overridden methods…

    void registerDrvOutput(const Realisation & info) override;

    void queryPathInfoUncached(const StorePath & path,
        Callback<std::shared_ptr<const ValidPathInfo>> callback) noexcept override;

    bool isValidPathUncached(const StorePath & path) override;

    void queryReferrers(const StorePath & path, StorePathSet & referrers) override;

    StorePathSet queryValidDerivers(const StorePath & path) override;

    std::optional<StorePath> queryPathFromHashPart(const std::string & hashPart) override;

    void registerValidPaths(const ValidPathInfos & infos) override;

    void addToStore(const ValidPathInfo & info, Source & source,
        RepairFlag repair, CheckSigsFlag checkSigs) override;

    StorePath addToStoreFromDump(Source & dump, std::string_view name,
        FileIngestionMethod method, HashType hashAlgo, RepairFlag repair, const StorePathSet & references) override;

    StorePath addTextToStore(
        std::string_view name,
        std::string_view s,
        const StorePathSet & references,
        RepairFlag repair) override;

    void queryRealisationUncached(const DrvOutput&,
        Callback<std::shared_ptr<const Realisation>> callback) noexcept override;
};

}
