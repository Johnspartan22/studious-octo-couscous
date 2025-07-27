#include "otpch.h"

#include "server.h"
#include "game.h"

#include "configmanager.h"
#include "scriptmanager.h"

#ifdef _MULTIPLATFORM77
#include "rsa.h"
#endif

#include "protocollogin.h"
#include "protocolstatus.h"
#include "databasemanager.h"
#include "scheduler.h"
#include "databasetasks.h"

#include <iostream>
#include <streambuf>
#include <sstream>
#include <cstdlib>
#include <cstdio>
#include <ctime>

DatabaseTasks g_databaseTasks;
Dispatcher g_dispatcher;
Scheduler g_scheduler;

IPList serverIPs;

Game g_game;
ConfigManager g_config;
Monsters g_monsters;
Vocations g_vocations;

#ifdef _MULTIPLATFORM77
RSA g_RSA;
#endif

std::mutex g_loaderLock;
std::condition_variable g_loaderSignal;
std::unique_lock<std::mutex> g_loaderUniqueLock(g_loaderLock);

// Define a null stream buffer that discards any output.
class NullBuffer : public std::streambuf {
public:
    int overflow(int c) override {
        return c; // Discard the character.
    }
};

// Your custom banner (keep this)
void printServerBanner()
{
    std::cout << "========================================" << std::endl;
    std::cout << "::          Underworld OT            ::" << std::endl;
    std::cout << "::           Version 7.6             ::" << std::endl;
    std::cout << "::         Created by Bezos          ::" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "     A reproduction of 1337 engine" << std::endl;
    std::cout << "========================================" << std::endl;
}

// Your enhanced status display (keep this)
void displayServerStatus()
{
    
}

// Modified error handling (merged style)
void startupErrorMessage(const std::string& errorStr)
{
    std::cout << "=====================================\n";
    std::cout << ">> STARTUP ERROR: " << errorStr << std::endl;
    std::cout << ">> Please check your configuration!\n";
    std::cout << "=====================================\n";
    g_loaderSignal.notify_all();
}

// Required from original
void badAllocationHandler()
{
    // Use functions that only use stack allocation
    puts("Allocation failed, server out of memory.\nDecrease the size of your map or compile in 64 bits mode.\n");
    getchar();
    exit(-1);
}

// Modified main loader with required functionality
void mainLoader(int argc, char* argv[], ServiceManager* services)
{
    // Set initial game state and seed random generator
    g_game.setGameState(GAME_STATE_STARTUP);
    srand(static_cast<unsigned int>(OTSYS_TIME()));

    #ifdef _WIN32
    SetConsoleTitle(STATUS_SERVER_NAME);
    system("color 0F");
    #endif

    // Print custom banner and additional server info
    printServerBanner();
    

std::cout << "The " << STATUS_SERVER_NAME << " Version: 1 " << std::endl;
    std::cout << "Compiled with: " << BOOST_COMPILER << std::endl;
    std::cout << "Compiled on " << __DATE__ << " " << __TIME__ << " for platform ";
    #if defined(__amd64__) || defined(_M_X64)
        std::cout << "x64" << std::endl;
    #elif defined(__i386__) || defined(_M_IX86) || defined(_X86_)
        std::cout << "x86" << std::endl;
    #elif defined(__arm__)
        std::cout << "ARM" << std::endl;
    #else
        std::cout << "unknown" << std::endl;
    #endif
    std::cout << std::endl;
    std::cout << "Welcome to the Underworld." << std::endl;
    std::cout << std::endl;

#ifdef _MULTIPLATFORM77
{
    const char* p("11966810357499309373149440509153068749966064890288435817723103352250031562738443964177490303450977003212332339404068541615495313743950332979149257144735789");
    const char* q("10996244068476141117665581873831610958435433430313128287686027579654923196336765989067158590180448533616147223668568372807055613865073980762330206837137907");
    g_RSA.setKey(p, q);
}
#endif

    // Load config
    std::cout << ":: Loading configuration... ";
    if (!g_config.load()) {
        startupErrorMessage("Unable to load config.lua!");
        return;
    }
    std::cout << "[done]" << std::endl;

    #ifdef _WIN32
    // Set process priority if configured
    const std::string& defaultPriority = g_config.getString(ConfigManager::DEFAULT_PRIORITY);
    if (strcasecmp(defaultPriority.c_str(), "high") == 0) {
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
    } else if (strcasecmp(defaultPriority.c_str(), "above-normal") == 0) {
        SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
    }
    #endif

    // Establish database connection
    std::cout << ":: Establishing database connection... " << std::flush;
    Database* db = Database::getInstance();
    if (!db->connect()) {
        startupErrorMessage("Failed to connect to database.");
        return;
    }
    std::cout << "[done]" << std::endl;

    // Run database manager and update database    
    if (!DatabaseManager::isDatabaseSetup()) {
        startupErrorMessage("The database you have specified in config.lua is empty, please import the schema.sql to your database.");
        return;
    }
    g_databaseTasks.start();
    DatabaseManager::updateDatabase();

    if (g_config.getBoolean(ConfigManager::OPTIMIZE_DATABASE) && !DatabaseManager::optimizeTables()) {
        // Optional: Add error handling for table optimization here.
    }

    // Load vocations
    std::cout << ":: Loading vocations... ";
    if (!g_vocations.loadFromXml()) {
        startupErrorMessage("Unable to load vocations!");
        return;
    }
    std::cout << "[done]" << std::endl;

    // Load items
    std::cout << ":: Loading items... ";
    if (Item::items.loadFromOtb("data/items/items.otb") != ERROR_NONE) {
        startupErrorMessage("Unable to load items (OTB)!");
        return;
    }
    if (!Item::items.loadFromXml()) {
        startupErrorMessage("Unable to load items (XML)!");
        return;
    }
    std::cout << "[done]" << std::endl;

    // Load script systems
    std::cout << ":: Loading scripts... ";
    if (!ScriptingManager::getInstance()->loadScriptSystems()) {
        startupErrorMessage("Failed to load script systems");
        return;
    }
    std::cout << "[done]" << std::endl;

    // Load monsters
    std::cout << ":: Loading monsters... ";
    if (!g_monsters.loadFromXml()) {
        startupErrorMessage("Unable to load monsters!");
        return;
    }
    std::cout << "[done]" << std::endl;

    // Check and set world type
    std::cout << ":: Checking world type... " << std::flush;
    std::string worldType = asLowerCaseString(g_config.getString(ConfigManager::WORLD_TYPE));
    if (worldType == "pvp") {
        g_game.setWorldType(WORLD_TYPE_PVP);
    } else if (worldType == "no-pvp") {
        g_game.setWorldType(WORLD_TYPE_NO_PVP);
    } else if (worldType == "pvp-enforced") {
        g_game.setWorldType(WORLD_TYPE_PVP_ENFORCED);
    } else {
        std::ostringstream ss;
        ss << "> ERROR: Unknown world type: " << g_config.getString(ConfigManager::WORLD_TYPE)
           << ", valid world types are: pvp, no-pvp and pvp-enforced.";
        startupErrorMessage(ss.str());
        return;
    }
    std::cout << asUpperCaseString(worldType) << std::endl;

    // Load main map silently (suppress extra output):
    std::cout << ":: Loading map... " << std::flush;
    // Save the current std::cout buffer.
    std::streambuf* oldCoutBuffer = std::cout.rdbuf();
    // Create a null buffer and redirect std::cout to it.
    NullBuffer nullBuffer;
    std::ostream nullStream(&nullBuffer);
    std::cout.rdbuf(nullStream.rdbuf());

    // Attempt to load the map (any output during this call is suppressed).
    bool mapLoaded = g_game.loadMainMap(g_config.getString(ConfigManager::MAP_NAME));

    // Restore the original std::cout buffer.
    std::cout.rdbuf(oldCoutBuffer);

    if (!mapLoaded) {
        startupErrorMessage("Failed to load map");
        return;
    }
    std::cout << "[done]" << std::endl;

    // Initialize gamestate
    std::cout << ":: Initializing gamestate..." << std::endl;
    g_game.setGameState(GAME_STATE_INIT);

    // Game client protocols
    services->add<ProtocolGame>(g_config.getNumber(ConfigManager::GAME_PORT));
    services->add<ProtocolLogin>(g_config.getNumber(ConfigManager::LOGIN_PORT));
    // OT protocols
    services->add<ProtocolStatus>(g_config.getNumber(ConfigManager::STATUS_PORT));

    // House rent: determine rent period and pay houses accordingly
    RentPeriod_t rentPeriod;
    std::string strRentPeriod = asLowerCaseString(g_config.getString(ConfigManager::HOUSE_RENT_PERIOD));
    if (strRentPeriod == "yearly") {
        rentPeriod = RENTPERIOD_YEARLY;
    } else if (strRentPeriod == "weekly") {
        rentPeriod = RENTPERIOD_WEEKLY;
    } else if (strRentPeriod == "monthly") {
        rentPeriod = RENTPERIOD_MONTHLY;
    } else if (strRentPeriod == "daily") {
        rentPeriod = RENTPERIOD_DAILY;
    } else {
        rentPeriod = RENTPERIOD_NEVER;
    }
    g_game.map.houses.payHouses(rentPeriod);

    // Final initialization message and server status display
    std::cout << ":: All files loaded." << std::endl;

    // Build the IP list for server connections
    std::pair<uint32_t, uint32_t> IpNetMask;
    IpNetMask.first = inet_addr("127.0.0.1");
    IpNetMask.second = 0xFFFFFFFF;
    serverIPs.push_back(IpNetMask);

    char szHostName[128];
    if (gethostname(szHostName, 128) == 0) {
        hostent* he = gethostbyname(szHostName);
        if (he) {
            unsigned char** addr = (unsigned char**)he->h_addr_list;
            while (addr[0] != nullptr) {
                IpNetMask.first = *(uint32_t*)(*addr);
                IpNetMask.second = 0x0000FFFF;
                serverIPs.push_back(IpNetMask);
                addr++;
            }
        }
    }

    std::string ip = g_config.getString(ConfigManager::IP);
    uint32_t resolvedIp = inet_addr(ip.c_str());
    if (resolvedIp == INADDR_NONE) {
        struct hostent* he = gethostbyname(ip.c_str());
        if (he != 0) {
            resolvedIp = *(uint32_t*)he->h_addr;
        } else {
            std::cout << "ERROR: Cannot resolve " << ip << "!" << std::endl;
            startupErrorMessage("");
            return;
        }
    }
    IpNetMask.first = resolvedIp;
    IpNetMask.second = 0;
    serverIPs.push_back(IpNetMask);

    #ifndef _WIN32
    if (getuid() == 0 || geteuid() == 0) {
        std::cout << "> Warning: " << STATUS_SERVER_NAME << " has been executed as root user, please consider running it as a normal user." << std::endl;
    }
    #endif

    // Start the game and change gamestate to normal
    g_game.start(services);
    g_game.setGameState(GAME_STATE_NORMAL);
    g_loaderSignal.notify_all();

    // Optionally display your enhanced server status
    displayServerStatus();
}

// Modified main function with required error handling
int main(int argc, char* argv[])
{
    // Setup bad allocation handler
    std::set_new_handler(badAllocationHandler);

    ServiceManager serviceManager;

    g_dispatcher.start();
    g_scheduler.start();

    g_dispatcher.addTask(createTask(std::bind(mainLoader, argc, argv, &serviceManager)));

    g_loaderSignal.wait(g_loaderUniqueLock);

    if (serviceManager.is_running()) {
        std::cout << ":: =========================::" << std::endl;
        std::cout << "::  Underworld OT is online ::" << std::endl;
        std::cout << ":: =========================::" << std::endl;
        serviceManager.run();
    } else {
        std::cout << ">> No services running. The server is NOT online." << std::endl;
        g_scheduler.shutdown();
        g_databaseTasks.shutdown();
        g_dispatcher.shutdown();
    }

    g_scheduler.join();
    g_databaseTasks.join();
    g_dispatcher.join();
    return 0;
}