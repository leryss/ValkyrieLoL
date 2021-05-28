#pragma once
#include "ScriptInfo.h"
#include "ScriptSubmission.h"
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

enum ScriptRepoColumns {
	REPO_COLUMN_ID,
	REPO_COLUMN_NAME,
	REPO_COLUMN_AUTHOR,
	REPO_COLUMN_CHAMP,
	REPO_COLUMN_RATING
};

class ScriptEntry {

public:
	/// Server version of script
	std::shared_ptr<ScriptInfo> remote;

	/// Locally stored version of script
	std::shared_ptr<ScriptInfo> local;

	/// Submission of the script
	std::shared_ptr<ScriptSubmission> submission;

	/// Current state of the local version of the script
	ScriptEntryState state;
};

struct ScriptRepoEntryComparator
{
public:
	ScriptRepoEntryComparator(std::map<std::string, std::shared_ptr<ScriptEntry>>& entries) 
		:entries(entries)
	{}

	bool operator()(const std::string &a, const std::string &b) const
	{
		if (sortSpecs == nullptr)
			return false;

		auto find = entries.find(a);
		if (find == entries.end())
			return false;
		auto& e1 = ExtractInfo(find->second);

		find = entries.find(b);
		if (find == entries.end())
			return false;
		auto& e2 = ExtractInfo(find->second);

		bool result;
		switch (sortSpecs->Specs->ColumnUserID) {
			case REPO_COLUMN_ID     : result = e1->id < e2->id                       ; break;
			case REPO_COLUMN_NAME 	: result = e1->name < e2->name                   ; break;
			case REPO_COLUMN_AUTHOR : result = e1->author < e2->author               ; break;
			case REPO_COLUMN_CHAMP  : result = e1->champion < e2->champion           ; break;
			case REPO_COLUMN_RATING : result = e1->averageRating < e2->averageRating ; break;
			default: result = false;
		}

		return (sortSpecs->Specs->SortDirection == ImGuiSortDirection_Ascending ? result : !result);
	}

	const std::shared_ptr<ScriptInfo>& ExtractInfo(const std::shared_ptr<ScriptEntry>& entry) const {
		if (entry->remote != nullptr)
			return entry->remote;
		else
			return entry->local;
	}

	std::map<std::string, std::shared_ptr<ScriptEntry>>& entries;
	ImGuiTableSortSpecs*                                 sortSpecs;
};

class ScriptRepository {

public:

	ScriptRepository();

	/// Loads the local script entries from disk
	void LoadLocalEntries(std::string path);

	/// Saves the local script entries to disk
	void SaveLocalEntries(std::string path);

	/// Sends a request to the server for the list of script entries and updates the repository on success
	void LoadRemoteEntries(const IdentityInfo& identity);

	void Draw();

	/// Adds an entry to the script index
	void AddEntry(std::shared_ptr<ScriptInfo>& script, bool isLocal);

	/// Removes and entry from the script index
	void RemoveEntry(std::string id, bool isLocal, bool isBoth);

	/// True when local entries were modified (added/removed)
	bool localsUnsaved = false;

	/// Map with all the script entries
	std::map<std::string, std::shared_ptr<ScriptEntry>> entries;

private:
	void DrawTable(bool showLocal);
	void DrawActions();
	void DrawScriptEdit();
	void SelectEntry(int selected);

	/// Sends a request to download the script code and installs the script locally on success
	void DownloadScriptAndInstall(std::shared_ptr<ScriptInfo> script);

	/// Reads the code from disk and submits an script update
	void SubmitScriptUpdate(std::shared_ptr<ScriptInfo> script);

	void UpdateSubmissions(std::vector<std::shared_ptr<ScriptSubmission>>& submissions);

	/// Writes the script file to disk and updates the entries index
	void InstallScript(std::shared_ptr<ScriptInfo> script, std::string& code);

	/// Removes the file from this and updates the entries index
	void UninstallScript(std::shared_ptr<ScriptInfo> script);

	/// Creates a new script locally. Create a new file with the base script code
	void CreateLocalEntry();

	/// Sorts the 'sorted' vector which is used to render the table view of the script entries
	void SortEntries();

	/// Updates the state of an entry
	void UpdateState(std::shared_ptr<ScriptEntry>& entry);

	void UpdateInstalledScripts();

	void HandleRating(std::shared_ptr<ScriptInfo>& local, std::shared_ptr<ScriptInfo>& remote);

private:

	/// Mutex for entries map. It is necessary because we retrieve some script entries asynchronously
	std::mutex                                          mtxEntries;
	
	/// List of script ids sorted (used for drawing the script table)
	std::vector<std::string>                            sorted;
	ScriptRepoEntryComparator                           comparator;
	char                                                searchStr[50];

	/// Index in sorted array of selected script
	int                                                 selectedScript = -1;

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

	static const int NumRatings = 5;
	static const char* Ratings[NumRatings];
	static const ImVec4 ColorRatings[NumRatings];
};