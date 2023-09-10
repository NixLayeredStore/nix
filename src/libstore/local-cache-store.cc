#include "archive.hh"
#include "local-cache-store.hh"
#include "callback.hh"
#include "url.hh"
#include <regex>

namespace nix {

LocalCacheStore::LocalCacheStore(const Params & params)
    : StoreConfig(params)
    , LocalFSStoreConfig(params)
    , LocalCacheStoreConfig(params)
    , Store(params)
    , LocalFSStore(params)
{
}

LocalCacheStore::LocalCacheStore(std::string scheme, std::string path, const Params & params)
    : LocalCacheStore(params)
{
    debug("LocalCacheStore");
    for (const auto& [key, value] : params)
        debug("params[%s] = %s", key, value);
    debug("rootDir: %s", this->rootDir.get());
    debug("realStoreDir: %s", this->realStoreDir.get());
}

void LocalCacheStore::narFromPath(const StorePath & path, Sink & sink)
{
    debug("narFromPath");
    if (!isValidPath(path))
        throw Error("path '%s' is not valid", printStorePath(path));
    auto pathToDump = std::string(printStorePath(path), storeDir.size());
    dumpPath(pathToDump, sink);
}

void LocalCacheStore::queryPathInfoUncached(
    const StorePath & path, Callback<std::shared_ptr<const ValidPathInfo>> callback) noexcept
{
    debug("queryPathInfoUncached");
    auto callbackPtr = std::make_shared<decltype(callback)>(std::move(callback));
    try
    {
        debug("LocalCacheStore.queryPathInfoUncached storePath: %s", printStorePath(path));
        auto narinfo_path = std::string{path.hashPart()} + ".narinfo";
        auto real_narinfo_path = this->realStoreDir.get() + narinfo_path;
        debug("LocalCacheStore.queryPathInfoUncached: %s", narinfo_path);
        try {
            auto narinfo_str = nix::readFile(real_narinfo_path);
            debug("LocalCacheStore.queryPathInfoUncached read success");
            debug(narinfo_str);
            debug(narinfo_path);
            auto validPathInfo = (std::shared_ptr<ValidPathInfo>)std::make_shared<NarInfo>(*this, narinfo_str, narinfo_path);
            return (*callbackPtr)(std::move(validPathInfo));
        } catch (SysError &) {
            debug("LocalCacheStore.queryPathInfoUncached error storePath: %s", printStorePath(path));
            return (*callbackPtr)({});
        }
    } catch (...) {
        debug("LocalCacheStore.queryPathInfoUncached error storePath: %s", printStorePath(path));
        return callbackPtr->rethrow();
    }
}

// Unimplemented methods
std::optional<StorePath> LocalCacheStore::queryPathFromHashPart(const std::string & hashPart)
{ unsupported("queryPathFromHashPart"); }
void LocalCacheStore::addToStore(const ValidPathInfo & info, Source & source,
                                  RepairFlag repair, CheckSigsFlag checkSigs)
{ unsupported("addToStore"); }
StorePath LocalCacheStore::addTextToStore(
    std::string_view name,
    std::string_view s,
    const StorePathSet & references,
    RepairFlag repair)
{ unsupported("addTextToStore"); }
void LocalCacheStore::queryRealisationUncached(const DrvOutput & drvOutput,
    Callback<std::shared_ptr<const Realisation>> callback) noexcept
{ unsupported("queryRealisationUncached"); }
std::optional<TrustedFlag> LocalCacheStore::isTrustedClient()
{ unsupported("isTrustedClient"); }
void LocalCacheStore::addIndirectRoot(const Path & path)
{ unsupported("addIndirectRoot"); }
Roots LocalCacheStore::findRoots(bool censor)
{ unsupported("findRoots"); }
void LocalCacheStore::collectGarbage(const GCOptions & options, GCResults & results)
{ unsupported("colelctGarbage"); }
void LocalCacheStore::addBuildLog(const StorePath & path, std::string_view log)
{ unsupported("addBuildLog"); }

static RegisterStoreImplementation<LocalCacheStore, LocalCacheStoreConfig> regLocalCacheStore;
}
