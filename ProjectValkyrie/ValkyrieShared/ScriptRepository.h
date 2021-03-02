#pragma once
#include "ScriptInfo.h"
#include "ValkyrieAPI.h"
#include "AsyncTaskPool.h"
#include <vector>
#include <mutex>

enum ScriptEntrySortType {
	SE_SORT_ID,
	SE_SORT_NAME,
	SE_SORT_STATE
};

enum ScriptEntryState {
	SE_STATE_UNINSTALLED,
	SE_STATE_INSTALLED,
	SE_STATE_OUTDATED,
	SE_STATE_CORRUPTED,
	SE_STATE_ONLY_LOCAL,

	SE_STATE_DOWNLOADING,
	SE_STATE_WAITING,
	SE_STATE_PENDING
};

class ScriptEntry {

public:
	/// Server version of script
	std::shared_ptr<ScriptInfo> remote;

	/// Locally stored version of script
	std::shared_ptr<ScriptInfo> local;

	/// Current state of the local version of the script
	ScriptEntryState state;
};

class ScriptRepository {

public:

	/// Loads the local script entries from disk
	void LoadLocalEntries(std::string path);

	/// Saves the local script entries to disk
	void SaveLocalEntries(std::string path);

	/// Sends a request to the server for the list of script entries and updates the repository on success
	void LoadRemoteEntries(const IdentityInfo& identity);

	void Draw();

	/// Adds an entry to the script index
	void AddEntry(ScriptInfo& script, bool isLocal);

	/// Removes and entry from the script index
	void RemoveEntry(std::string id, bool isLocal, bool isBoth);

	/// True when local entries were modified (added/removed)
	bool localsUnsaved = false;

private:
	void DrawTable(bool showLocal);
	void DrawActions();
	void DrawScriptEdit();
	void SelectEntry(int selected);

	/// Sends a request to download the script code and installs the script locally on success
	void DownloadScriptAndInstall(ScriptInfo& script);

	/// Writes the script file to disk and updates the entries index
	void InstallScript(ScriptInfo& script, std::string& code);

	/// Removes the file from this and updates the entries index
	void UninstallScript(ScriptInfo& script);

	/// Creates a new script locally. Create a new file with the base script code
	void CreateLocalEntry();

	/// Sorts the 'sorted' vector which is used to render the table view of the script entries
	void SortEntries();

	/// Updates the state of an entry
	void UpdateState(std::shared_ptr<ScriptEntry>& entry);

private:

	/// Mutex for entries map. It is necessary because we retrieve some script entries asynchronously
	std::mutex                                          mtxEntries;
	
	/// List of script ids sorted (used for drawing the script table)
	std::vector<std::string>                            sorted;

	/// Index in sorted array of selected script
	int                                                 selectedScript = -1;

	/// Map with all the script entries
	std::map<std::string, std::shared_ptr<ScriptEntry>> entries;

	IdentityInfo identity;
	ValkyrieAPI* api        = ValkyrieAPI::Get();
	AsyncTaskPool* taskPool = AsyncTaskPool::Get();

	/// Buffers for creating new scripts
	const static int SIZE_BUFF = 300;
	char idBuff[SIZE_BUFF];
	char champBuff[SIZE_BUFF];
	char descBuff[SIZE_BUFF];
	char nameBuff[SIZE_BUFF];

	static std::string BaseCodeScript;
};