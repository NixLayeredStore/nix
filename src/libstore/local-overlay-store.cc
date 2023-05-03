#include "local-overlay-store.hh"
#include "callback.hh"


namespace nix {


std::string LocalOverlayStoreConfig::doc()
{
    return
        #include "local-overlay-store.md"
        ;
}


LocalOverlayStore::LocalOverlayStore(const Params & params)
    : StoreConfig(params)
    , LocalFSStoreConfig(params)
    , LocalStoreConfig(params)
    , LocalOverlayStoreConfig(params)
    , Store(params)
    , LocalFSStore(params)
    , LocalStore(params)
{
    throw UnimplementedError("LocalOverlayStore");
}


LocalOverlayStore::LocalOverlayStore(std::string scheme, std::string path, const Params & params)
    : LocalOverlayStore(params)
{
    //throw UnimplementedError("LocalOverlayStore");

    // TODO: Mount overlayfs
}


LocalOverlayStore::~LocalOverlayStore()
{
    // throw UnimplementedError("~LocalOverlayStore");

    // TODO: Unmount overlayfs
}


std::string LocalOverlayStore::getUri()
{
    return "overlay";
}


void LocalOverlayStore::registerDrvOutput(const Realisation & info)
{
    //throw UnimplementedError("~LocalOverlayStore");

    // TODO: queryRealisation(info.id)

    return LocalStore::registerDrvOutput(info);
}

void LocalOverlayStore::queryPathInfoUncached(const StorePath & path,
    Callback<std::shared_ptr<const ValidPathInfo>> callback) noexcept
{
    /*
    try {
        // TODO: Try upper and then lower in turn.
        // If in lower but not upper, copy up.

        throw UnimplementedError("queryPathInfoUncached");

    } catch (...) { callback.rethrow(); }
    */
    return LocalStore::queryPathInfoUncached(path, std::move(callback));
}


bool LocalOverlayStore::isValidPathUncached(const StorePath & path)
{
    // TODO: Try upper and then lower in turn.
    // If in lower but not upper, copy up.

    //throw UnimplementedError("isValidPathUncached");

    return LocalStore::isValidPathUncached(path);
}


StorePathSet LocalOverlayStore::queryAllValidPaths()
{
    // TODO: Query both upper and lower and union the results.
    // Also insert the union into the upper so records can be referenced from foreign keys.
    //throw UnimplementedError("queryAllValidPaths");

    return LocalStore::queryAllValidPaths();
}


void LocalOverlayStore::queryReferrers(const StorePath & path, StorePathSet & referrers)
{
    // TODO: Query both upper and lower and union the results.
    // Also insert the union into the upper so records can be referenced from foreign keys.
    //throw UnimplementedError("queryReferrers");

    return LocalStore::queryReferrers(path, referrers);
}


StorePathSet LocalOverlayStore::queryValidDerivers(const StorePath & path)
{
    // TODO: Query both upper and lower and union the results.
    // Also insert the union into the upper so records can be referenced from foreign keys.
    //throw UnimplementedError("queryValidDerivers");

    return LocalStore::queryValidDerivers(path);
}


std::map<std::string, std::optional<StorePath>>
LocalOverlayStore::queryPartialDerivationOutputMap(const StorePath & path_)
{
    // TODO: Query both upper and lower and union the results.
    // Also insert the union into the upper so records can be referenced from foreign keys.
    //throw UnimplementedError("queryPartialDerivationOutputMap");

    return LocalStore::queryPartialDerivationOutputMap(path_);
}

std::optional<StorePath> LocalOverlayStore::queryPathFromHashPart(const std::string & hashPart)
{
    // TODO: Something like queryPathInfoUncached (not quite since return type of this function is StorePath)
    //throw UnimplementedError("queryPathFromHashPart");

    return LocalStore::queryPathFromHashPart(hashPart);
}


void LocalOverlayStore::registerValidPaths(const ValidPathInfos & infos)
{
    //throw UnimplementedError("registerValidPaths");

    // TODO: queryPathInfo(p) for each path p
    // This should ensure all paths are copied to upper db so they are available
    // for the internal query functions that registerValidPaths will call on the upper db.

    return LocalStore::registerValidPaths(infos);
}


void LocalOverlayStore::repairPath(const StorePath & path)
{
    // TODO: queryPathInfo(path)
    // To ensure paths are in upper db ...

    return LocalStore::repairPath(path);
}


Roots LocalOverlayStore::findRoots(bool censor)
{
    //throw UnimplementedError("findRoots");

    //Roots roots;

    //// TODO: Warn that garbage collection is not supported.

    //return roots;

    return LocalStore::findRoots(censor);
}


void LocalOverlayStore::collectGarbage(const GCOptions & options, GCResults & results)
{
    //throw UnimplementedError("collectGarbage");

    // TODO: Warn that garbage collection is not supported.

    return LocalStore::collectGarbage(options, results);
}

void LocalOverlayStore::optimiseStore()
{
    //throw UnimplementedError("optimiseStore");

    // TODO: Warn that store optimisation is not supported.

    return LocalStore::optimiseStore();
}


bool LocalOverlayStore::verifyStore(bool checkContents, RepairFlag repair)
{
    //throw UnimplementedError("verifyStore");

    // TODO: Warn that verifying the store is not supported.

    //return false;

    return LocalStore::verifyStore(checkContents, repair);
}


void LocalOverlayStore::addSignatures(const StorePath & storePath, const StringSet & sigs)
{
    // TODO: queryPathInfo(storePath)
    // To ensure the paths are copied to the upper db.

    //throw UnimplementedError("addSignatures");

    return LocalStore::addSignatures(storePath, sigs);
}


void LocalOverlayStore::queryRealisationUncached(const DrvOutput & id,
        Callback<std::shared_ptr<const Realisation>> callback) noexcept
{
    // throw UnimplementedError("queryRealisationUncached");

    //try {
    //    // TODO: Try upper and then lower in turn.
    //    // If in lower but not upper, copy up.

    //} catch (...) {
    //    callback.rethrow();
    //}

    return LocalStore::queryRealisationUncached(id, std::move(callback));
}


static RegisterStoreImplementation<LocalOverlayStore, LocalStoreConfig> regLocalStore;


}  // namespace nix
