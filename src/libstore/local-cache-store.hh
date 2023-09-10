#include "local-store.hh"

namespace nix {

/**
 * Configuration for `LocalCacheStore`.
 */
struct LocalCacheStoreConfig : virtual LocalFSStoreConfig
{
    LocalCacheStoreConfig(const StringMap & params)
        : StoreConfig(params)
        , LocalFSStoreConfig(params)
    { }

    const std::string name() override { return "Experimental Local Cache Store"; }

    std::string doc() override
    {
        return
          ""
          // FIXME write docs
          //#include "local-cache-store.md"
          ;
    }

};

/**
 * Variation of local store that uses a using overlayfs for the store dir.
 */
class LocalCacheStore : public virtual LocalCacheStoreConfig, public virtual LocalFSStore
{

public:
    LocalCacheStore(const Params & params);

    LocalCacheStore(std::string scheme, std::string path, const Params & params);

    static std::set<std::string> uriSchemes()
    { return { "local-cache" }; }

    std::string getUri() override
    {
        return "local-cache";
    }

private:
    // Overridden methods…

    void narFromPath(const StorePath & path, Sink & sink) override;

    void queryPathInfoUncached(const StorePath & path,
        Callback<std::shared_ptr<const ValidPathInfo>> callback) noexcept override;

    std::optional<StorePath> queryPathFromHashPart(const std::string & hashPart);

    void addToStore(const ValidPathInfo & info, Source & source,
                                     RepairFlag repair, CheckSigsFlag checkSigs);

    StorePath addTextToStore(
        std::string_view name,
        std::string_view s,
        const StorePathSet & references,
        RepairFlag repair);

    void queryRealisationUncached(const DrvOutput & drvOutput,
                                  Callback<std::shared_ptr<const Realisation>> callback) noexcept;

    std::optional<TrustedFlag> isTrustedClient();
    void addIndirectRoot(const Path & path);
    Roots findRoots(bool censor);
    void collectGarbage(const GCOptions & options, GCResults & results);
    void addBuildLog(const StorePath & path, std::string_view log);

};

}
