#pragma once

static const char* s_defaultCustomId = "SampleLoginCustomId";
static const char* s_logFileName = "PFSampleLog.txt";

class PFUtils
{
public:
    PFUtils();
    ~PFUtils() = default;

    void RunPlayFabSDKSample(const std::string& title);

    void WriteLogToFile(const char* strIn, const char* strFileName);

private:
    std::string m_customId;
    std::string m_titleId;
    std::string m_endpoint;

    void ExitIfFailed(HRESULT hr, const char* func);
    void ExitIfPartyError(PartyError error, const char* func);
    void ExitIfMultiplayerError(HRESULT hr, const char* func);

	void SetEndpoint();
};

