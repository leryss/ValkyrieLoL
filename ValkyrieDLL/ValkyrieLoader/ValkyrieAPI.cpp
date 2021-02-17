#include "ValkyrieAPI.h"

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
}

AuthResponse ValkyrieAPI::Authorize(const char * name, const char * password, float durationSecs)
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

	AuthResponse response;
	SendLambdaRequest(&response, payload);

	response.token = lambdaRawResponse.View().GetString("body");

	return response;
}

void ValkyrieAPI::SendLambdaRequest(BaseAPIResponse* response, std::shared_ptr<Aws::IOStream>& payload)
{
	Aws::Lambda::Model::InvokeRequest req;
	req.SetFunctionName("valkyrie-api");
	req.SetInvocationType(Aws::Lambda::Model::InvocationType::RequestResponse);
	req.SetLogType(Aws::Lambda::Model::LogType::Tail);
	req.SetBody(payload);
	req.SetContentType("application/javascript");

	auto outcome = lambdaClient->Invoke(req);
	if (!outcome.IsSuccess()) {
		response->success = false;
		response->error = "Failed to connect to server";
	}

	lambdaRawResponse = Aws::Utils::Json::JsonValue(outcome.GetResult().GetPayload());
	response->success = lambdaRawResponse.View().GetInteger("code") == 200 ? true : false;
	if (!response->success)
		response->error = lambdaRawResponse.View().GetString("error");
}
