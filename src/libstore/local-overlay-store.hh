#pragma once
///@file

#include "sqlite.hh"

#include "pathlocks.hh"
#include "store-api.hh"
#include "local-store.hh"
#include "gc-store.hh"
#include "sync.hh"
#include "util.hh"
//#include "ref.hh"

#include <chrono>
#include <future>
#include <string>
#include <unordered_set>


namespace nix {


struct LocalOverlayStoreConfig : virtual LocalStoreConfig
{
    using LocalStoreConfig::LocalStoreConfig;

    const Setting<std::string> lowerUri{(StoreConfig*) this,
        "",
        "lower",
        "Uri of the underlying Nix store."};

    const PathSetting workDir{(StoreConfig*) this,
        false,
        rootDir + "/nix/.store",
        "work",
        "Directory to use as overlayfs workdir."};

    const std::string name() override { return "Local Overlay Store"; }

    std::string doc() override;
};


class LocalOverlayStore : public virtual LocalOverlayStoreConfig,
    public virtual LocalStore,
    public virtual GcStore
{
    std::shared_ptr<LocalFSStore> lowerStore;

public:

    LocalOverlayStore(const Params & params, std::string lowerUri = "auto");
    LocalOverlayStore(std::string scheme, std::string path, const Params & params);

    ~LocalOverlayStore();

    static std::set<std::string> uriSchemes()
    { return {"overlay"}; }

    std::string getUri() override;

    bool isValidPathUncached(const StorePath & path) override;

    StorePathSet queryAllValidPaths() override;

    void queryPathInfoUncached(const StorePath & path,
        Callback<std::shared_ptr<const ValidPathInfo>> callback) noexcept override;

    void queryReferrers(const StorePath & path, StorePathSet & referrers) override;

    StorePathSet queryValidDerivers(const StorePath & path) override;

    std::map<std::string, std::optional<StorePath>> queryPartialDerivationOutputMap(const StorePath & path) override;

    std::optional<StorePath> queryPathFromHashPart(const std::string & hashPart) override;

    Roots findRoots(bool censor) override;

    void collectGarbage(const GCOptions & options, GCResults & results) override;

    void optimiseStore() override;

    bool verifyStore(bool checkContents, RepairFlag repair) override;

    void registerValidPaths(const ValidPathInfos & infos) override;

    void repairPath(const StorePath & path) override;

    void addSignatures(const StorePath & storePath, const StringSet & sigs) override;

    void registerDrvOutput(const Realisation & info) override;

    void queryRealisationUncached(const DrvOutput&,
        Callback<std::shared_ptr<const Realisation>> callback) noexcept override;

};


}
