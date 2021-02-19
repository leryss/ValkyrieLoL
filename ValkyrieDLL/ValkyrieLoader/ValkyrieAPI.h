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

using namespace Aws::Lambda;

enum APIRequestStatus {
	RS_EXECUTING,
	RS_SUCCESS,
	RS_FAILURE
};

class BaseAPIResponse {
public:
	APIRequestStatus status = RS_SUCCESS;
	Aws::String      error;

	void OnFinish(Model::InvokeOutcome& outcome);
protected:
	Aws::Utils::Json::JsonValue lambdaRawResponse;
};

class GetUserInfoResponse: public BaseAPIResponse {
public:
	UserInfo user;

	InvokeResponseReceivedHandler onFinishHandler = [this](const LambdaClient* client, const Model::InvokeRequest& req, Model::InvokeOutcome outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& ctx){
		this->OnFinish(outcome);
		if(status == RS_SUCCESS)
			user = UserInfo::FromJsonView(lambdaRawResponse.View().GetObject("result"));
	};
};

class GetUserListInfoResponse : public BaseAPIResponse {
public:
	std::vector<UserInfo> users;

	InvokeResponseReceivedHandler onFinishHandler = [this](const LambdaClient* client, const Model::InvokeRequest& req, Model::InvokeOutcome outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& ctx) {
		this->OnFinish(outcome);
		if (status == RS_SUCCESS) {
			auto jsonUsers = lambdaRawResponse.View().GetArray("result");
			int numUsers = jsonUsers.GetLength();
			for (int i = 0; i < numUsers; ++i) {
				auto jsonUser = jsonUsers.GetItem(i);
				users.push_back(UserInfo::FromJsonView(jsonUser));
			}
		}
	};
};

class GetS3ObjectResponse : public BaseAPIResponse {
public:
	Aws::S3::Model::GetObjectResult result;

	Aws::S3::GetObjectResponseReceivedHandler onFinishHandler = [this](const Aws::S3::S3Client* client, const Aws::S3::Model::GetObjectRequest& req, Aws::S3::Model::GetObjectOutcome outcome, const std::shared_ptr<const Aws::Client::AsyncCallerContext>& ctx) {
		if (!outcome.IsSuccess()) {
			status = RS_FAILURE;
			error = "Failed to retrieve S3 object";
		}
		else {
			status = RS_SUCCESS;
			result = (Aws::S3::Model::GetObjectResult&&)outcome.GetResult();
		}
	};
};



class ValkyrieAPI {

public:
	ValkyrieAPI();

	std::shared_ptr<GetUserListInfoResponse> GetUsers(const IdentityInfo& identity);
	std::shared_ptr<GetUserInfoResponse>     GetUser(const IdentityInfo& identity, const char* target);
	std::shared_ptr<GetS3ObjectResponse>     GetCheatS3Object(const char* bucket, const char* key);
private:

	Aws::String apiToken;

	std::shared_ptr<Aws::Lambda::LambdaClient> lambdaClient;
	std::shared_ptr<Aws::S3::S3Client>         s3Client;

	Aws::Lambda::Model::InvokeRequest          lambdaRequest;
};