#pragma once

#include <aws/core/Aws.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>

#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/GetObjectResult.h>
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

class AsyncLambdaInvoke: public AsyncTask {

public:

	AsyncLambdaInvoke(LambdaClient& lambdaClient, Model::InvokeRequest& req)
      :lambda(lambdaClient),
       request(req)
	{}

	virtual void Perform() {
		
		currentStep = "Invoking AWS Lambda";

		Model::InvokeOutcome outcome = lambda.Invoke(request);
		if (!outcome.IsSuccess()) {
			SetStatus(ASYNC_FAILED);
			error = "Failed to invoke lambda";
		}
		else {
			rawJson = Aws::Utils::Json::JsonValue(outcome.GetResult().GetPayload());
			int code = rawJson.View().GetInteger("code");
			if(code == 200)
				SetStatus(ASYNC_SUCCEEDED);
			else {
				SetStatus(ASYNC_FAILED);
				error = rawJson.View().GetString("error").c_str();
			}
		}
	}

	Aws::Utils::Json::JsonValue rawJson;
	LambdaClient&               lambda;
	Model::InvokeRequest&       request;
};

class GetUserListAsync : public AsyncLambdaInvoke {
public:
	std::vector<UserInfo> users;

	using AsyncLambdaInvoke::AsyncLambdaInvoke;

	virtual void Perform() {
		AsyncLambdaInvoke::Perform();

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

class UserOperationAsync : public AsyncLambdaInvoke {
public:
	UserInfo user;

	using AsyncLambdaInvoke::AsyncLambdaInvoke;

	virtual void Perform() {
		AsyncLambdaInvoke::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED) {
			user = UserInfo::FromJsonView(rawJson.View().GetObject("result"));
		}
	}
};

class GenerateInviteAsync : public AsyncLambdaInvoke {
public:
	Aws::String inviteCode;

	using AsyncLambdaInvoke::AsyncLambdaInvoke;

	virtual void Perform() {
		AsyncLambdaInvoke::Perform();

		if (GetStatus() == ASYNC_SUCCEEDED)
			inviteCode = rawJson.View().GetString("result");
	}
};

class ValkyrieAPI {

public:
	ValkyrieAPI();

	std::shared_ptr<GetS3ObjectAsync>     GetCheatS3Object(const char* bucket, const char* key);

	std::shared_ptr<UserOperationAsync>   CreateAccount(const char* name, const char* pass, const char* discord, const HardwareInfo& hardware, const char* inviteCode);

	std::shared_ptr<GetUserListAsync>     GetUsers(const IdentityInfo& identity);
	std::shared_ptr<UserOperationAsync>   GetUser(const IdentityInfo& identity, const char* target);
	std::shared_ptr<GenerateInviteAsync>  GenerateInviteCode(const IdentityInfo& identity, float days, UserLevel level);

	std::shared_ptr<UserOperationAsync>   UpdateUser(const IdentityInfo& identity, const char* target, const UserInfo& targetInfo);
	

private:

	Aws::String apiToken;

	std::shared_ptr<Aws::Lambda::LambdaClient> lambdaClient;
	std::shared_ptr<Aws::S3::S3Client>         s3Client;

	Model::InvokeRequest                       lambdaInvokeRequest;
	Aws::S3::Model::GetObjectRequest           s3GetObjectRequest;
};