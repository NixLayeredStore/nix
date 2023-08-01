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

    const PathSetting upperLayer{(StoreConfig*) this, "", "upper-layer",
        R"(
          Must be used as OverlayFS upper layer for this store's store dir.
        )"};

    Setting<bool> checkMount{(StoreConfig*) this, true, "check-mount",
        R"(
          Check that the overlay filesystem is correctly mounted.

          Nix does not manage the overlayfs mount point itself, but the correct
          functioning of the overlay store does depend on this mount point being set up
          correctly. Rather than just assume this is the case, check that the lowerdir
          and upperdir options are what we expect them to be. This check is on by
          default, but can be disabled if needed.
        )"};

    const PathSetting remountHook{(StoreConfig*) this, "", "remount-hook",
        R"(
          Script or other executable to run when overlay filesystem needs remounting.

          This is occasionally necessary when deleting a store path that exists in both upper and lower layers.
          In such a situation, bypassing OverlayFS and deleting the path in the upper layer directly
          is the only way to perform the deletion without creating a "whiteout".
          However this causes the OverlayFS kernel data structures to get out-of-sync,
          and can lead to 'stale file handle' errors; remounting solves the problem.

          The store directory is passed as an argument to the invoked executable.
        )"};

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

    void queryRealisationUncached(const DrvOutput&,
        Callback<std::shared_ptr<const Realisation>> callback) noexcept override;

    void collectGarbage(const GCOptions & options, GCResults & results) override;

    void deleteStorePath(const Path & path, uint64_t & bytesFreed) override;

    void optimiseStore() override;

    /**
     * Check all paths registered in the upper DB.
     *
     * Note that this includes store objects that reside in either overlayfs layer;
     * just enumerating the contents of the upper layer would skip them.
     * 
     * We don't verify the contents of both layers on the assumption that the lower layer is far bigger,
     * and also the observation that anything not in the upper db the overlayfs doesn't yet care about.
     */
    bool verifyAllValidPaths(RepairFlag repair, StorePathSet & validPaths) override;

    /**
     * For lower-store paths, we used the lower store location. This avoids the
     * wasteful "copying up" that would otherwise happen.
     */
    Path toRealPathForHardLink(const StorePath & storePath) override;

    /**
     * Deletion only effects the upper layer, so we ignore lower-layer referrers.
     */
    void queryGCReferrers(const StorePath & path, StorePathSet & referrers) override;

    void remountIfNecessary();

    std::atomic_bool _remountRequired = false;
};

}
