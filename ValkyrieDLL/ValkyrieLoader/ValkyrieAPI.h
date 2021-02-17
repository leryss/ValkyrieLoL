#pragma once

#include <aws/core/Aws.h>
#include <aws/lambda/LambdaClient.h>
#include <aws/lambda/model/InvokeRequest.h>

#include <aws/s3/S3Client.h>

#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/client/ClientConfiguration.h>

class OperationResponse {};

class BaseAPIResponse {
public:
	bool        success = true;
	Aws::String error;
};

class AuthResponse: public BaseAPIResponse {
public:
	Aws::String token;
};

class ValkyrieAPI {

public:
	ValkyrieAPI();

	AuthResponse Authorize(const char* name, const char* password, float durationSecs);
private:
	void SendLambdaRequest(BaseAPIResponse* response, std::shared_ptr<Aws::IOStream>& payload);

	Aws::Utils::Json::JsonValue lambdaRawResponse;
	Aws::String apiToken;

	std::shared_ptr<Aws::Lambda::LambdaClient> lambdaClient;
	std::shared_ptr<Aws::S3::S3Client>         s3Client;
};