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

	lambdaInvokeRequest.SetFunctionName("valkyrie-api");
	lambdaInvokeRequest.SetInvocationType(Aws::Lambda::Model::InvocationType::RequestResponse);
	lambdaInvokeRequest.SetLogType(Aws::Lambda::Model::LogType::Tail);
	lambdaInvokeRequest.SetContentType("application/javascript");
}

std::shared_ptr<UserOperationAsync> ValkyrieAPI::CreateAccount(const char * name, const char * pass, const char * discord, const HardwareInfo & hardware, const char * inviteCode)
{
	std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("PayloadCreateAcc");

	Aws::Utils::Json::JsonValue jsonParams;
	jsonParams.WithString("name", name);
	jsonParams.WithString("pass", pass);
	jsonParams.WithString("discord", discord);
	jsonParams.WithString("invite-code", inviteCode);
	jsonParams.WithObject("hardware", hardware.ToJsonValue());

	Aws::Utils::Json::JsonValue json;
	json.WithString("operation", "create-account");
	json.WithObject("operation-params", jsonParams);

	*payload << json.View().WriteReadable();
	lambdaInvokeRequest.SetBody(payload);

	return std::shared_ptr<UserOperationAsync>(new UserOperationAsync(*lambdaClient, lambdaInvokeRequest));
}

std::shared_ptr<GetUserListAsync> ValkyrieAPI::GetUsers(const IdentityInfo & identity)
{
	std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("PayloadGetUser");

	Aws::Utils::Json::JsonValue jsonParams;
	jsonParams.WithString("name", identity.name.c_str());
	jsonParams.WithString("pass", identity.pass.c_str());
	jsonParams.WithObject("hardware", identity.hardware.ToJsonValue());

	Aws::Utils::Json::JsonValue json;
	json.WithString("operation", "list-users");
	json.WithObject("operation-params", jsonParams);

	*payload << json.View().WriteReadable();
	lambdaInvokeRequest.SetBody(payload);

	return std::shared_ptr<GetUserListAsync>(new GetUserListAsync(*lambdaClient, lambdaInvokeRequest));
}

std::shared_ptr<UserOperationAsync> ValkyrieAPI::GetUser(const IdentityInfo & identity, const char* target)
{
	std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("PayloadGetUser");

	Aws::Utils::Json::JsonValue jsonParams;
	jsonParams.WithString("name", identity.name.c_str());
	jsonParams.WithString("pass", identity.pass.c_str());
	jsonParams.WithObject("hardware", identity.hardware.ToJsonValue());
	jsonParams.WithString("target", target);

	Aws::Utils::Json::JsonValue json;
	json.WithString("operation", "get-user");
	json.WithObject("operation-params", jsonParams);

	*payload << json.View().WriteReadable();
	lambdaInvokeRequest.SetBody(payload);

	return std::shared_ptr<UserOperationAsync>(new UserOperationAsync(*lambdaClient, lambdaInvokeRequest));
}

std::shared_ptr<GenerateInviteAsync> ValkyrieAPI::GenerateInviteCode(const IdentityInfo & identity, float days)
{
	std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("PayloadGenerateIdentity");

	Aws::Utils::Json::JsonValue jsonParams;
	jsonParams.WithString("name", identity.name.c_str());
	jsonParams.WithString("pass", identity.pass.c_str());
	jsonParams.WithDouble("days", days);
	jsonParams.WithObject("hardware", identity.hardware.ToJsonValue());

	Aws::Utils::Json::JsonValue json;
	json.WithString("operation", "generate-invite");
	json.WithObject("operation-params", jsonParams);

	*payload << json.View().WriteReadable();
	lambdaInvokeRequest.SetBody(payload);

	return std::shared_ptr<GenerateInviteAsync>(new GenerateInviteAsync(*lambdaClient, lambdaInvokeRequest));
}

std::shared_ptr<UserOperationAsync> ValkyrieAPI::UpdateUser(const IdentityInfo & identity, const char * target, const UserInfo & targetInfo)
{
	std::shared_ptr<Aws::IOStream> payload = Aws::MakeShared<Aws::StringStream>("PayloadGetUser");

	Aws::Utils::Json::JsonValue jsonParams;
	jsonParams.WithString("name", identity.name.c_str());
	jsonParams.WithString("pass", identity.pass.c_str());
	jsonParams.WithObject("hardware", identity.hardware.ToJsonValue());
	jsonParams.WithString("target", target);
	jsonParams.WithObject("target-info", targetInfo.ToJsonValue());

	Aws::Utils::Json::JsonValue json;
	json.WithString("operation", "update-user");
	json.WithObject("operation-params", jsonParams);

	*payload << json.View().WriteReadable();
	lambdaInvokeRequest.SetBody(payload);

	return std::shared_ptr<UserOperationAsync>(new UserOperationAsync(*lambdaClient, lambdaInvokeRequest));
}

std::shared_ptr<GetS3ObjectAsync> ValkyrieAPI::GetCheatS3Object(const char* bucket, const char* key)
{
	s3GetObjectRequest.SetBucket(bucket);
	s3GetObjectRequest.SetKey(key);

	return std::shared_ptr<GetS3ObjectAsync>(new GetS3ObjectAsync(*s3Client, s3GetObjectRequest));
}