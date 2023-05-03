#include "local-overlay-store.hh"
#include "callback.hh"
#include <iostream>

#include <curl/curl.h>


#include <sys/mount.h>


namespace nix {


std::string LocalOverlayStoreConfig::doc()
{
    return
        #include "local-overlay-store.md"
        ;
}

namespace {
    std::string urlDecode(const std::string & input) {
        static CURL * curl = nullptr;  // From version 7.82.0 onwards, the curl argument can be null.
        if (!curl) curl = curl_easy_init();
        return std::string{curl_easy_unescape(curl, input.c_str(), input.length(), nullptr)};
    }
}

        //return std::string{curl_easy_unescape(curl, "overlay%3A%2F%2F%2Ftmp%2Ftest-store", 35, nullptr)};
        //std::cerr << "\x1b[1;36mlower = '\x1b[0;36m" << lower << "\x1b[1;36m'\x1b[0m" << std::endl;

LocalOverlayStore::LocalOverlayStore(const Params & params, std::string lowerUri)
    : StoreConfig(params)
    , LocalFSStoreConfig(params)
    , LocalStoreConfig(params)
    , LocalOverlayStoreConfig(params)
    , Store(params)
    , LocalFSStore(params)
    , LocalStore(params)
    , lowerStore(openStore(lowerUri).dynamic_pointer_cast<LocalFSStore>())
{
    if (!lowerStore)
        throw Error("lower store must be local: %s", lowerUri);

    auto lowerDir = lowerStore->realStoreDir.get();
    auto upperDir = realStoreDir.get();

    auto options = std::string("lowerdir=") + lowerDir + ","
                 + std::string("upperdir=") + upperDir + ","
                 + std::string("workdir=") + workDir.get();

    std::cerr << "options: " << options << std::endl;

    createDirs(workDir.get());

    if (mount("overlay", upperDir.c_str(), "overlay", 0, options.c_str()) == -1)
        throw SysError("cannot mount overlay filesystem: %s", lowerUri);
}

/*
    outputs/out/bin/nix-store --verify --store "overlay://$(echo "/tmp/test/lower" | jq -Rr @uri)?root=/tmp/test/upper"
    nix-build --store "overlay://$(echo "/tmp/test/lower" | jq -Rr @uri)?root=/tmp/test/upper" -E '
        let pkgs = import <nixpkgs> { }; in pkgs.nix
    '


    strace -f -e trace=mount outputs/out/bin/nix-build --store "overlay://auto?root=/tmp/test/upper&state=/nix/var/nix" -E 'let pkgs = import <nixpkgs> { }; in pkgs.openssl'


    sudo strace -f -e trace=mount -s 1024 "$PWD/outputs/out/bin/nix-build" --store "overlay://auto?root=/tmp/test/upper" -E 'let pkgs = import <nixpkgs> { }; in pkgs.openssl'
*/


LocalOverlayStore::LocalOverlayStore(std::string scheme, std::string path, const Params & params)
    : LocalOverlayStore(params, urlDecode(path))
{
    std::cerr << "LocalOverlayStore()" << std::endl;
    std::cerr << "    scheme = '" << scheme << "'" << std::endl;
    std::cerr << "    path = '" << path << "'" << std::endl;
    std::cerr << "    params = {" << std::endl;
    for (const auto& [k, v] : params) {
        std::cerr << k << ": '" << v << "'" << std::endl;
    }
    std::cerr << "    }" << std::endl;
}


LocalOverlayStore::~LocalOverlayStore()
{
    auto target = realStoreDir.get().c_str();

    if (umount2(target, MNT_DETACH) == -1)
        std::cerr << "cannot unmount overlay filesystem: " << target << std::endl;

    std::cerr << "unmounted overlay filesystem: " << target << std::endl;
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
    //std::shared_ptr<const ValidPathInfo> queryPathInfoInternal(State & state, const StorePath & path);

    //lowerStore->

    auto callbackPtr = std::make_shared<decltype(callback)>(std::move(callback));

    return LocalStore::queryPathInfoUncached(path, {
        [this, path = path, callbackPtr = callbackPtr](std::future<std::shared_ptr<const ValidPathInfo>> fut) {
            try {
                auto info = fut.get();

                if (info) {
                    return (*callbackPtr)(std::move(info));
                }

                //return lowerStore->queryPathInfoUncached(path, {
                //    [this, path = path, callbackPtr = callbackPtr](std::future<std::shared_ptr<const ValidPathInfo>> fut) {
                //        //try 
                //    }
                //});

                /*
                if (diskCache)
                    diskCache->upsertNarInfo(getUri(), hashPart, info);

                {
                    auto state_(state.lock());
                    state_->pathInfoCache.upsert(std::string(storePath.to_string()), PathInfoCacheValue { .value = info });
                }

                if (!info || !goodStorePath(storePath, info->path)) {
                    stats.narInfoMissing++;
                    throw InvalidPath("path '%s' is not valid", printStorePath(storePath));
                }

                (*callbackPtr)(ref<const ValidPathInfo>(info));
                */

            } catch (...) {
                // TODO: Consider how to properly compose exception handling.
                callbackPtr->rethrow();
            }
        }
    });

    //LocalStore::queryPathInfoUncached(path, )

    /*
    try {
        // TODO: Try upper and then lower in turn.
        // If in lower but not upper, copy up.

        throw UnimplementedError("queryPathInfoUncached");

    } catch (...) { callback.rethrow(); }
    */


    //return LocalStore::queryPathInfoUncached(path, std::move(callback));
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
    std::cerr << "LocalOverlayStore::verifyStore" << std::endl;

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


static RegisterStoreImplementation<LocalOverlayStore, LocalOverlayStoreConfig> regLocalOverlayStore;


}  // namespace nix
