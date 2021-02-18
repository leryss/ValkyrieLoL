#include "ValkyrieAPI.h"
#include <thread>

ValkyrieAPI::ValkyrieAPI()
{
	Aws::SDKOptions options;
	Aws::InitAPI(options);

	Aws::Auth::AWSCredentials credentials;
	credentials.SetAWSAccessKeyId("AKIAU6GDVTT2BO5OFOE5");
	credentials.SetAWSSecretKey("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");

	Aws::Client::ClientConfiguration config;
	config.region = Aws::String("eu-north-1");

	lambdaClient = Aws::MakeShared<Aws::Lambda::LambdaClient>("lambda_client_tag", credentials, config);
	s3Client     = Aws::MakeShared<Aws::S3::S3Client>("s3_client_tag", credentials, config);

	lambdaRequest.SetFunctionName("valkyrie-api");
	lambdaRequest.SetInvocationType(Aws::Lambda::Model::InvocationType::RequestResponse);
	lambdaRequest.SetLogType(Aws::Lambda::Model::LogType::Tail);
	lambdaRequest.SetContentType("application/javascript");
}

std::shared_ptr<AuthResponse> ValkyrieAPI::Authorize(const char * name, const char * password, float durationSecs)
{
	std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("PayloadAuthorize");

	Aws::Utils::Json::JsonValue jsonHardware;
	jsonHardware.WithString("cpu", "wtf");

	Aws::Utils::Json::JsonValue jsonParams;
	jsonParams.WithString("name", name);
	jsonParams.WithString("pass", password);
	jsonParams.WithDouble("duration", durationSecs);
	jsonParams.WithObject("hardware", jsonHardware);

	Aws::Utils::Json::JsonValue json;
	json.WithString("operation", "authorize");
	json.WithObject("operation-params", jsonParams);

	*payload << json.View().WriteReadable();
	lambdaRequest.SetBody(payload);

	std::shared_ptr<AuthResponse> response(new AuthResponse());
	response->status = RS_EXECUTING;

	lambdaClient->InvokeAsync(lambdaRequest, response->onFinishHandler);
	return response;
}

std::shared_ptr<GetS3ObjectResponse> ValkyrieAPI::GetCheatS3Object(const char* bucket, const char* key)
{
	std::shared_ptr<GetS3ObjectResponse> response(new GetS3ObjectResponse());

	Aws::S3::Model::GetObjectRequest req;
	req.SetBucket(bucket);
	req.SetKey(key);

	s3Client->GetObjectAsync(req, response->onFinishHandler);
	response->status = RS_EXECUTING;
	return response;
}

void BaseAPIResponse::OnFinish(Model::InvokeOutcome& outcome)
{
	if (!outcome.IsSuccess()) {
		status = RS_FAILURE;
		error = "Failed to connect to server";
	}

	lambdaRawResponse = Aws::Utils::Json::JsonValue(outcome.GetResult().GetPayload());
	status = lambdaRawResponse.View().GetInteger("code") == 200 ? RS_SUCCESS : RS_FAILURE;
	if (status == RS_FAILURE)
		error = lambdaRawResponse.View().GetString("error");
}
