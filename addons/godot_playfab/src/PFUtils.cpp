#include "pch.h"
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#ifdef HC_PLATFORM == HC_PLATFORM_GDK
#include <XGameRuntimeInit.h>
#endif // HC_PLATFORM_GDK
#include <PFUtils.h>

using namespace Party;

PFUtils::PFUtils()
{
    m_customId = s_defaultCustomId;
}

void PFUtils::RunPlayFabSDKSample(const std::string& title)
{
    m_titleId = title;
    std::ostringstream oss;
    oss << "TitleID: " << m_titleId << "\r\n";
    godot::String TID = "TitleID:99DA\n";
    godot::UtilityFunctions::print(TID);
    oss << "CustomID: " << m_customId << "\r\n";
    godot::String CLI = "CustomID:SampleLoginCustomId\n";
    godot::UtilityFunctions::print(CLI);
    WriteLogToFile(oss.str().c_str(), s_logFileName);

    SetEndpoint();

    WriteLogToFile("=====================================================\r\n", s_logFileName);
	WriteLogToFile("Initialize SDKs:\r\n", s_logFileName);

    // Initialize PlayFabCore
#ifdef PLAYFAB_SAMPLE_SWITCH
    HRESULT hr = InitializePF();
#elif HC_PLATFORM == HC_PLATFORM_GDK
    HRESULT hr = XGameRuntimeInitialize();    
    ExitIfFailed(hr, "XGameRuntimeInitialize");
    hr = PFInitialize(nullptr);
#endif
    ExitIfFailed(hr, "PFInitialize");

    // Initialize PFServices
#if defined(PLAYFAB_SAMPLE_PLAYSTATION5) || defined(PLAYFAB_SAMPLE_PLAYSTATION4)
    hr = PFServicesInitialize();
#else
    hr = PFServicesInitialize(nullptr);
#endif // !PLAYFAB_SAMPLE_PLAYSTATION5 && !PLAYFAB_SAMPLE_PLAYSTATION4
    ExitIfFailed(hr, "PFServicesInitialize");

    PFServiceConfigHandle serviceHandle{ nullptr };
    hr = PFServiceConfigCreateHandle(m_endpoint.c_str(), m_titleId.c_str(), &serviceHandle);
    ExitIfFailed(hr, "PFServiceConfigCreateHandle");
    if (serviceHandle == NULL)
    {
        WriteLogToFile("Failure to create ServiceConfigHandle. Exiting. \r\n", s_logFileName);
        exit(1);
    }

    // Multiplayer Initialize
    MultiplayerInitializationConfiguration multiplayerInitConfig = {};
    multiplayerInitConfig.titleId = m_titleId.c_str();
    multiplayerInitConfig.multiplayerTaskQueue = nullptr;
    PFMultiplayerHandle multiplayerHandle;
    hr = PFMultiplayerInitialize(&multiplayerInitConfig, &multiplayerHandle);
	ExitIfMultiplayerError(hr, "PFMultiplayerInitialize");

    // Party Initialize
    PartyManager& partyManager = PartyManager::GetSingleton();
    PartyInitializationConfiguration partyInitConfig = {};
    partyInitConfig.titleId = m_titleId.c_str();
    PartyError err;

    err = partyManager.Initialize(&partyInitConfig);
	ExitIfPartyError(err, "PartyManager Initialize");

    WriteLogToFile("=====================================================\r\n", s_logFileName);
    WriteLogToFile("Login to PlayFab:\r\n", s_logFileName);

	// Get Player Using CustomID
    std::shared_ptr<Player> player = Player::GetPlayer();
    assert(player);

    PFAuthenticationLoginWithCustomIDRequest request{};
    request.createAccount = true;
    request.customId = m_customId.c_str();

	// Log Player with LoginWithCustomIDRequest
    XAsyncBlock async{};
    hr = PFAuthenticationLoginWithCustomIDAsync(serviceHandle, &request, &async);
    hr = XAsyncGetStatus(&async, true);

    // Prepare login result buffer 
    std::vector<char> loginResultBuffer;
    PFAuthenticationLoginResult const* loginResult;
    size_t bufferSize;
    hr = PFAuthenticationLoginWithCustomIDGetResultSize(&async, &bufferSize);
    loginResultBuffer.resize(bufferSize);

    // Create EntityHandle 
    PFEntityHandle entityHandle{ nullptr };
    hr = PFAuthenticationLoginWithCustomIDGetResult(&async, &entityHandle, loginResultBuffer.size(), loginResultBuffer.data(), &loginResult, nullptr);
    ExitIfFailed(hr, "Login with PFAuthenticationLoginWithCustomIDGetResult");
    
	// Initialize Party Local User
    PartyLocalUser* localUser{};
    err = partyManager.CreateLocalUser(entityHandle, &localUser);
	ExitIfPartyError(err, "Party CreateLocalUser");

    WriteLogToFile("=====================================================\r\n", s_logFileName);
    WriteLogToFile("Cleanup SDKs:\r\n", s_logFileName);

	// Cleanup PartyManager
    err = partyManager.Cleanup();
    ExitIfPartyError(err, "Party Cleanup");

    // Cleanup Multiplayer
    hr = PFMultiplayerUninitialize(multiplayerHandle);
	ExitIfMultiplayerError(hr, "PFMultiplayerUninitialize");

    // Cleanup PFServices
    PFEntityCloseHandle(entityHandle);
    entityHandle = nullptr;

    PFServiceConfigCloseHandle(serviceHandle);
    serviceHandle = nullptr;

    // Clean up PFCore
    XAsyncBlock asyncPFCore{};
	hr = PFUninitializeAsync(&asyncPFCore);
    hr = XAsyncGetStatus(&asyncPFCore, true);
    ExitIfFailed(hr, "PFUninitializeAsync");

	// Clean up PFServices
    XAsyncBlock asyncPFServices{};
	PFServicesUninitializeAsync(&asyncPFServices);
    hr = XAsyncGetStatus(&asyncPFServices, true);
    ExitIfFailed(hr, "PFServicesUninitializeAsync");

    XGameRuntimeUninitialize();

    godot::String Done = "Test run finished\n";
    godot::UtilityFunctions::print(Done);
    WriteLogToFile("Test run finished\r\n", s_logFileName);
    return;
}

void PFUtils::ExitIfFailed(HRESULT hrin, const char* func)
{
    if (FAILED(hrin))
    {
        std::ostringstream oss;
        oss << "HRESULT returned: 0x" << std::hex << hrin;
        std::string errMsg = std::string(func) + " failed! " + oss.str() + "\r\n";
        godot::String Fail = "Test run FAILED\n";
        godot::UtilityFunctions::print(Fail);
        WriteLogToFile(errMsg.c_str(), s_logFileName);
        exit(1);
    }
    else
    {
        std::string msg = std::string(func) + " succeed \r\n";
        WriteLogToFile(msg.c_str(), s_logFileName);
        godot::String Success = godot::String(func) + " succeed\n";
        godot::UtilityFunctions::print(Success);
    }
 }

void PFUtils::ExitIfPartyError(PartyError error, const char* func)
{
    PartyString errorMessage;
    if (PARTY_FAILED(error))
    {
        PartyGetErrorMessage(error, &errorMessage);
        std::string errMsg = std::string(func) + " failed! \r\n";
        WriteLogToFile(errMsg.c_str(), s_logFileName);
        WriteLogToFile(errorMessage, s_logFileName);
        exit(1);
    }
    else
    {
        std::string msg = std::string(func) + " succeed \r\n";
        WriteLogToFile(msg.c_str(), s_logFileName);
    }
}

void PFUtils::ExitIfMultiplayerError(HRESULT hrin, const char* func)
{
    if (FAILED(hrin))
    {
        std::ostringstream oss;
        oss << "HRESULT returned: 0x" << std::hex << PFMultiplayerGetErrorMessage(hrin);
        std::string errMsg = std::string(func) + " failed! " + oss.str() + "\r\n";
        WriteLogToFile(errMsg.c_str(), s_logFileName);
        exit(1);
    }
    else
    {
        std::string msg = std::string(func) + " succeed \r\n";
        WriteLogToFile(msg.c_str(), s_logFileName);
    }
}

void PFUtils::SetEndpoint()
{
    std::ostringstream endpointStream;
    endpointStream << "https://" << m_titleId << ".playfabapi.com";
    m_endpoint = endpointStream.str();
}

void PFUtils::WriteLogToFile(const char* strIn, const char* strFileName)
{
#if defined(PLAYFAB_SAMPLE_PLAYSTATION5) || defined(PLAYFAB_SAMPLE_PLAYSTATION4)
    std::cout << strIn << std::endl;
#elif PLAYFAB_SAMPLE_SWITCH
    OutputLog(strIn);
#elif HC_PLATFORM == HC_PLATFORM_GDK
    UNREFERENCED_PARAMETER(strFileName);
    AllocConsole();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, strIn, -1, NULL, 0);
    wchar_t* lpBuff = new wchar_t[size_needed];
    MultiByteToWideChar(CP_UTF8, 0, strIn, -1, lpBuff, size_needed);

    DWORD dwSize = 0;
    WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), lpBuff, lstrlenW(lpBuff), &dwSize, NULL);

    delete[] lpBuff;
#else
    HANDLE hFile;
    std::string str = strIn;
    DWORD dwBytesToWrite = (DWORD)str.length();
    DWORD dwBytesWritten = 0;
    BOOL bErrorFlag = FALSE;

    static std::vector<std::string> firstLogLinePerFile{};
    DWORD dwCreationDisposition = OPEN_ALWAYS;
    if (std::find(firstLogLinePerFile.begin(), firstLogLinePerFile.end(), strFileName) == firstLogLinePerFile.end())
    {
        firstLogLinePerFile.push_back(strFileName);
        dwCreationDisposition = CREATE_ALWAYS; // recreate log upon start
    }

    char szPath[MAX_PATH];
    GetModuleFileNameA(NULL, szPath, MAX_PATH);
    std::string strFullPath = szPath;
    size_t pos = strFullPath.find_last_of("\\/");
    strFullPath = strFullPath.substr(0, pos);
    strFullPath += "\\";
    strFullPath += strFileName;
    hFile = CreateFileA(strFullPath.c_str(), FILE_APPEND_DATA, 0, NULL, dwCreationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return;
    }

    bErrorFlag = WriteFile(
        hFile,           // open file handle
        str.data(),      // start of data to write
        dwBytesToWrite,  // number of bytes to write
        &dwBytesWritten, // number of bytes that were written
        NULL);            // no overlapped structure

    if (!bErrorFlag && dwBytesWritten != dwBytesToWrite)
    {
        OutputDebugStringA("Log file error: dwBytesWritten != dwBytesToWrite\n");
    }

    CloseHandle(hFile);

    OutputDebugStringA(strIn);

    std::cout << strIn << std::endl;
#endif // !PLAYFAB_SAMPLE_PLAYSTATION5 && !PLAYFAB_SAMPLE_PLAYSTATION4
}