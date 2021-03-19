#pragma once

#include <mutex>

#include <aws/core/Aws.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>

#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/GetObjectResult.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/HeadObjectResult.h>
#include <aws/s3/S3Client.h>

#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/ClientConfiguration.h>

#include "Models.h"
#include "AsyncTask.h"


using namespace Aws::Lambda;

class GetS3ObjectAsync : public AsyncTask {

public:
	GetS3ObjectAsync(Aws::S3::S3Client& s3Client, Aws::S3::Model::GetObjectRequest& request)
		:s3Client(s3Client), request(request)
	{}

	virtual void Perform() {
		
		currentStep = "Requesting S3 Object";
		auto outcome = s3Client.GetObject(request);

		if (!outcome.IsSuccess()) {
			SetStatus(ASYNC_FAILED);
			error = "Failed to get s3 object";
		}
		else {
			SetStatus(ASYNC_SUCCEEDED);
			result = (Aws::S3::Model::GetObjectResult&&)outcome.GetResult();
		}
	}

	Aws::S3::Model::GetObjectResult   result;
	Aws::S3::S3Client&                s3Client;
	Aws::S3::Model::GetObjectRequest& request;
};

class GetS3ObjectHeadResultAsync : public AsyncTask {

public:
	GetS3ObjectHeadResultAsync(Aws::S3::S3Client& s3Client, Aws::S3::Model::HeadObjectRequest& request)
		:s3Client(s3Client), request(request)
	{}

	virtual void Perform() {

		currentStep = "Requesting object head";
		
		auto outcome = s3Client.HeadObject(request);

		if (!outcome.IsSuccess()) {
			SetStatus(ASYNC_FAILED);
			error = "Failed to get object heading";
		}
		else {
			SetStatus(ASYNC_SUCCEEDED);
			result = (Aws::S3::Model::HeadObjectResult&&)outcome.GetResult();
		}
	}

	Aws::S3::Model::HeadObjectResult   result;
	Aws::S3::S3Client&                 s3Client;
	Aws::S3::Model::HeadObjectRequest& request;

	
};

class LambdaInvokeResultAsync: public AsyncTask {

public:

	LambdaInvokeResultAsync(LambdaClient& lambdaClient, Model::InvokeRequest req)
      :lambda(lambdaClient),
       request(req)
	{}

	virtual void Perform() {
		
		currentStep = "Invoking AWS Lambda";
		
		RequestMutex.lock();
		Model::InvokeOutcome outcome = lambda.Invoke(request);
		RequestMutex.unlock();

		if (!outcome.IsSuccess()) {
			SetStatus(ASYNC_FAILED);
			error = "Failed to invoke lambda";
		}
		else {
			rawJson = Aws::Utils::Json::JsonValue(outcome.GetResult().GetPayload());
			auto jsonView = rawJson.View();

			if (!jsonView.KeyExists("code")) {
				SetStatus(ASYNC_FAILED);
				error = "Server side error. Please report this error to the admins";
			}
			else {
				int code = jsonView.GetInteger("code");
				if (code == 200)
					SetStatus(ASYNC_SUCCEEDED);
				else {
					SetStatus(ASYNC_FAILED);
					error = rawJson.View().GetString("error").c_str();
				}
			}
		}
	}

	Aws::Utils::Json::JsonValue rawJson;
	LambdaClient&               lambda;
	Model::InvokeRequest        request;

	static std::mutex RequestMutex;
};

class GetUserListAsync : public LambdaInvokeResultAsync {
public:
	std::vector<UserInfo> users;

	using LambdaInvokeResultAsync::LambdaInvokeResultAsync;

	virtual void Perform() {
		LambdaInvokeResultAsync::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED) {
			auto jsonUsers = rawJson.View().GetArray("result");
			int numUsers = jsonUsers.GetLength();

			for (int i = 0; i < numUsers; ++i) {
				auto jsonUser = jsonUsers.GetItem(i);
				users.push_back(UserInfo::FromJsonView(jsonUser));
			}
		}
	}
};

class UserResultAsync : public LambdaInvokeResultAsync {
public:
	UserInfo user;

	using LambdaInvokeResultAsync::LambdaInvokeResultAsync;

	virtual void Perform() {
		LambdaInvokeResultAsync::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED) {
			user = UserInfo::FromJsonView(rawJson.View().GetObject("result"));
		}
	}
};

class StringResultAsync : public LambdaInvokeResultAsync {
public:
	Aws::String result;

	using LambdaInvokeResultAsync::LambdaInvokeResultAsync;

	virtual void Perform() {
		LambdaInvokeResultAsync::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED)
			result = rawJson.View().GetString("result");
	}
};

class ScriptListAsync : public LambdaInvokeResultAsync {

public:
	std::vector<std::shared_ptr<ScriptInfo>> scripts;

	using LambdaInvokeResultAsync::LambdaInvokeResultAsync;

	virtual void Perform() {
		LambdaInvokeResultAsync::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED) {
			auto jsonScripts = rawJson.View().GetArray("result");
			int numScripts = jsonScripts.GetLength();

			for (int i = 0; i < numScripts; ++i) {
				auto jsonScript = jsonScripts.GetItem(i);
				scripts.push_back(ScriptInfo::FromJsonView(jsonScript));
			}
		}
	}
};

class ScriptSubmissionsResultAsync : public LambdaInvokeResultAsync {
public:
	std::vector<std::shared_ptr<ScriptSubmission>> submissions;

	using LambdaInvokeResultAsync::LambdaInvokeResultAsync;

	virtual void Perform() {
		LambdaInvokeResultAsync::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED) {
			auto jsonScripts = rawJson.View().GetArray("result");
			for (size_t i = 0; i < jsonScripts.GetLength(); ++i) {
				auto jsonSubmission = jsonScripts.GetItem(i);
				submissions.push_back(ScriptSubmission::FromJsonView(jsonSubmission));
			}
		}
	}
};

class InviteListResultAsync : public LambdaInvokeResultAsync {
public:
	std::vector<std::shared_ptr<InviteInfo>> invites;

	using LambdaInvokeResultAsync::LambdaInvokeResultAsync;

	virtual void Perform() {
		LambdaInvokeResultAsync::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED) {
			auto inviteListJson = rawJson.View().GetArray("result");
			for (size_t i = 0; i < inviteListJson.GetLength(); ++i) {
				auto inviteJson = inviteListJson.GetItem(i);
				invites.push_back(InviteInfo::FromJsonView(inviteJson));
			}
		}
	}
};

class ValkyrieAPI {

public:
	std::shared_ptr<GetS3ObjectHeadResultAsync>   GetS3ObjectHead(const char* bucket, const char* key);
	std::shared_ptr<GetS3ObjectAsync>             GetCheatS3Object(const char* bucket, const char* key);
										          
	std::shared_ptr<UserResultAsync>              CreateAccount(const char* name, const char* pass, const char* discord, const HardwareInfo& hardware, const char* inviteCode);
	std::shared_ptr<GetUserListAsync>             GetUsers(const IdentityInfo& identity);
	std::shared_ptr<UserResultAsync>              GetUser(const IdentityInfo& identity, const char* target);
	std::shared_ptr<InviteListResultAsync>        GenerateInviteCodes(const IdentityInfo & identity, const InviteInfo& invite, int numToGenerate);
	std::shared_ptr<UserResultAsync>              UpdateUser(const IdentityInfo& identity, const char* target, const UserInfo& targetInfo);
										          
	std::shared_ptr<ScriptListAsync>              GetScriptList(const IdentityInfo& identity);
	std::shared_ptr<StringResultAsync>            GetScriptCode(const IdentityInfo& identity, std::string& id);

	std::shared_ptr<ScriptSubmissionsResultAsync> SubmitScript(const IdentityInfo& identity, const ScriptInfo& script, const std::string& code);
	std::shared_ptr<ScriptSubmissionsResultAsync> GetSubmissions(const IdentityInfo& identity, const std::string& name);
	std::shared_ptr<ScriptSubmissionsResultAsync> GetAllSubmissions(const IdentityInfo& identity);

	std::shared_ptr<LambdaInvokeResultAsync>      UpdateSubmission(const IdentityInfo& identity, const ScriptSubmission& submission);
	std::shared_ptr<StringResultAsync>            ExtendSubscription(const char* name, const char* code);

	static ValkyrieAPI*                   Get();

private:
	void                                  PutIdentity(JsonValue& json, const IdentityInfo& identity);
	void                                  PutOperation(const char* operation, JsonValue& params);

private:

	ValkyrieAPI();
	static ValkyrieAPI*                        Instance;
				                               
	std::shared_ptr<Aws::Lambda::LambdaClient> lambdaClient;
	std::shared_ptr<Aws::S3::S3Client>         s3Client;

	Model::InvokeRequest                       lambdaInvokeRequest;
	Aws::S3::Model::HeadObjectRequest          s3HeadObjectRequest;
	Aws::S3::Model::GetObjectRequest           s3GetObjectRequest;
};