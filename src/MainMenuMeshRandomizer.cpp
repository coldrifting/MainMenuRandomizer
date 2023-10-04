#include <PCH.h>
#include <stddef.h>

#include <spdlog/sinks/basic_file_sink.h>

#define toLog SKSE::log::info

using std::string;
using std::vector;

void SetupLog() {
    auto logsFolder = SKSE::log::log_directory();
    if (!logsFolder) SKSE::stl::report_and_fail("SKSE log_directory not provided, logs disabled.");
    auto pluginName = SKSE::PluginDeclaration::GetSingleton()->GetName();
    auto logFilePath = *logsFolder / std::format("{}.log", pluginName);
    auto fileLoggerPtr = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath.string(), true);
    auto loggerPtr = std::make_shared<spdlog::logger>("log", std::move(fileLoggerPtr));
    spdlog::set_default_logger(std::move(loggerPtr));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::trace);
}

int random(int min, int max) {
    std::random_device seed;
    std::mt19937 gen{seed()};
    std::uniform_int_distribution<> dist{min, max};
    return dist(gen);
}

string toLower(string str) {
    string str2 = str;
    for (int i = 0; i < str.length(); i++) 
        str2[i] = tolower(str2[i]);

    return str2;
}

std::vector<std::string> getPaths()
{
    string logoPath = "Interface/MainMenu/";

    vector<string> paths = vector<string>();

    string dir = "Data/Meshes/" + logoPath;

    if (!std::filesystem::exists(dir)) {
        return paths;
    }

    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::filesystem::path filepath = entry.path();
            string filename = filepath.filename().string();
            if (toLower(filename).ends_with(".nif")) {
                toLog("Found Main main replacer: {}", filename);
                paths.push_back(logoPath + filename);
            }
        }
    }
    
    return paths;
}

string getRandomModel() {
    vector<string> paths = getPaths();

    if (paths.empty()) {
        toLog("No Main menu replacers found. Using default...");
        return "";
    }

    string newModelPath = paths[random(0, paths.size() - 1)];

    toLog("Replacing Main Menu Icon with {}", newModelPath);

    return newModelPath;
}

struct Hook {
    static RE::BSFadeNode* RequestModel1(const char* modelPath, const void* unk1) {
        string newModel = getRandomModel();
        if (newModel == "") {
            return RequestModel1Original(modelPath, unk1);
        }

        return RequestModel1Original(newModel.c_str(), unk1);
    }

    static RE::BSFadeNode* RequestModel2(const char* modelPath, const void* unk1) {
        string newModel = getRandomModel();
        if (newModel == "") {
            return RequestModel2Original(modelPath, unk1);
        }

        return RequestModel2Original(newModel.c_str(), unk1);
    }

    inline static REL::Relocation<decltype(RequestModel1)> RequestModel1Original;
    inline static REL::Relocation<decltype(RequestModel2)> RequestModel2Original;
};

void Install() {
    toLog("Initializing Trampoline...");

    SKSE::Trampoline& trampoline = SKSE::GetTrampoline();
    trampoline.create(42);  // Each write_call allocates 14 bytes

    toLog("Trampoline Initialized.");

    // SE & AE Addresses and offsets for the function that places the main menu logo
    // In this case we care about the call to RequestModel2_*
    toLog("Patching Main Menu Logo...");
    uintptr_t addr = REL::RelocationID(51445, 52304).address();
    uintptr_t offset = REL::Relocate(0x245, 0x250);
    Hook::RequestModel2Original = trampoline.write_call<5>(addr + offset, Hook::RequestModel2);

    // Sneaky call to RequestModel_* I almost missed that only happens on main menu reloads
    uintptr_t addr2 = REL::RelocationID(51455, 52316).address();
    uintptr_t offset2 = REL::Relocate(0x91, 0xA0);
    Hook::RequestModel1Original = trampoline.write_call<5>(addr2 + offset2, Hook::RequestModel1);

    // If on AE, apply patch for AE Upgrade
    // TODO - Test this somehow? I don't feel like shelling out $20 so...
    if (REL::Module::IsAE()) {
        toLog("AE Detected. Patching AE Upgrade Main Menu Logo...");
        uintptr_t ae_addr = REL::RelocationID(0, 52319).address();  // Not valid for SE
        uintptr_t ae_offset = 0x167;
        Hook::RequestModel2Original = trampoline.write_call<5>(ae_addr + ae_offset, Hook::RequestModel2);
    }
}

SKSEPluginLoad(const SKSE::LoadInterface* skse) {
    SetupLog();

    auto* plugin = SKSE::PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    toLog("{} {} is loading...", plugin->GetName(), version);

    Init(skse);
    Install();

    toLog("{} has finished loading.", plugin->GetName());
    return true;
}