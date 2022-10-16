#include "SimpleIni.h"
#include "include/Hooks.h"
#include "include/Papyrus.h"
#include "include/data.h"
#include "include/debuffHandler.h"
#include "include/events.h"
#include "include/lib/TrueHUDAPI.h"
#include "ValhallaCombat.hpp"
#include "include/settings.h"
#include "include/Utils.h"
#include "include/lib/ValhallaCombatAPI.h"
#include "include/ModAPI.h"
void initTrueHUDAPI() {
	auto val = ValhallaCombat::GetSingleton();
	val->ersh_TrueHUD = reinterpret_cast<TRUEHUD_API::IVTrueHUD3*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
	if (val->ersh_TrueHUD) {
		logger::info("Obtained TruehudAPI - {0:x}", (uintptr_t)val->ersh_TrueHUD);
		settings::facts::TrueHudAPI_Obtained = true;
	} else {
		logger::info("TrueHUD API not found.");
		settings::facts::TrueHudAPI_Obtained = false;
	}
}

void initPrecisionAPI() {
	auto val = ValhallaCombat::GetSingleton();
	val->ersh_Precision = reinterpret_cast<PRECISION_API::IVPrecision1*>(PRECISION_API::RequestPluginAPI(PRECISION_API::InterfaceVersion::V1));
	if (val->ersh_Precision) {
		logger::info("Obtained PrecisionAPI - {0:x}", (uintptr_t)val->ersh_Precision);
		settings::facts::PrecisionAPI_Obtained = true;
		auto res = val->ersh_Precision->AddPreHitCallback(SKSE::GetPluginHandle(), blockHandler::precisionPrehitCallbackFunc);
		if (res == PRECISION_API::APIResult::OK || res == PRECISION_API::APIResult::AlreadyRegistered) {
			logger::info("Collision prehit callback successfully registered.");
		}
	} else {
		logger::info("Precision API not found.");
		settings::facts::PrecisionAPI_Obtained = false;
	}
}

void onDataLoaded() 
{
	Hooks::install();  //hook install has been postponed for compatibility with other plugins.

	settings::init();
	settings::readSettings();
	events::registerAllEventHandlers();
	data::loadData();
	EldenCounterCompatibility::attemptInit();
	ValgrindCompatibility::attemptInit();
}

void onPostLoad() {
	logger::info("Initializing API fetch...");
	initTrueHUDAPI();
	initPrecisionAPI();
	logger::info("...done");
}

void onPostLoadGame() {
	debuffHandler::GetSingleton()->stopDebuff(RE::PlayerCharacter::GetSingleton());
	stunHandler::GetSingleton()->reset();
	settings::updateGlobals();
}



void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		onDataLoaded();
		break;
	case SKSE::MessagingInterface::kPostLoad:
		onPostLoad();
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
		onPostLoadGame();
		break;
	}
}

void onSKSEInit()
{
	Papyrus::Register();
}

namespace
{
	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path) {
			util::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log"sv, Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%g(%#): [%^%l%$] %v"s);
	}
}

std::string wstring2string(const std::wstring& wstr, UINT CodePage)

{

	std::string ret;

	int len = WideCharToMultiByte(CodePage, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);

	ret.resize((size_t)len, 0);

	WideCharToMultiByte(CodePage, 0, wstr.c_str(), (int)wstr.size(), &ret[0], len, NULL, NULL);

	return ret;

}


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Query(const SKSE::QueryInterface* a_skse, SKSE::PluginInfo* a_info)
{
	a_info->infoVersion = SKSE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	if (a_skse->IsEditor()) {
		logger::critical("Loaded in editor, marking as incompatible"sv);
		return false;
	}

	const auto ver = a_skse->RuntimeVersion();
	if (ver < SKSE::RUNTIME_SSE_1_5_39) {
		logger::critical(FMT_STRING("Unsupported runtime version {}"), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []() {
	SKSE::PluginVersionData v;

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);

	v.UsesAddressLibrary();
	v.UsesStructsPost629();
	v.CompatibleVersions({ SKSE::RUNTIME_SSE_LATEST });

	return v;
}();


extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
    #ifndef NDEBUG
	while (!IsDebuggerPresent()) { Sleep(100); }
#endif
	InitializeLog();
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	SKSE::Init(a_skse);

	auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}
	
	onSKSEInit();


	return true;
}


extern "C" DLLEXPORT void* SKSEAPI RequestPluginAPI(const VAL_API::InterfaceVersion a_interfaceVersion)
{
	//auto api = Messaging::TrueHUDInterface::GetSingleton();
	auto api = ModAPI::VALInterface::GetSingleton();
	logger::info("ValhallaCombat::RequestPluginAPI called");

	switch (a_interfaceVersion) {
	case VAL_API::InterfaceVersion::V1:
	case VAL_API::InterfaceVersion::V2:
		logger::info("ValhallaCombat::RequestPluginAPI returned the API singleton");
		return static_cast<void*>(api);
	}
	logger::info("ValhallaCombat::RequestPluginAPI requested the wrong interface version");
	return nullptr;
}
