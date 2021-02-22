#include "HardwareInfo.h"
#include <comdef.h>
#include <Wbemidl.h>

std::string GetWMIPropertyFrom(IWbemServices * pSvc, const wchar_t * propertyName, const wchar_t * tableName)
{
	wchar_t buffQuery[256];
	swprintf_s(buffQuery, L"SELECT %s FROM %s", propertyName, tableName);

	IEnumWbemClassObject* pEnumerator = NULL;
	pSvc->ExecQuery(
		bstr_t("WQL"),
		buffQuery,
		WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
		NULL,
		&pEnumerator);

	std::string result;
	if (pEnumerator != NULL) {
		IWbemClassObject *pclsObj = NULL;
		ULONG uReturn = 0;

		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		VARIANT vtProp;
		hr = pclsObj->Get(propertyName, 0, &vtProp, 0, 0);

		char asciiBuff[256];
		size_t size;
		wcstombs_s(&size, asciiBuff, vtProp.bstrVal, 256);
		result.append(asciiBuff);

		VariantClear(&vtProp);
		pclsObj->Release();
		pEnumerator->Release();
	}

	return result;
}


HardwareInfo HardwareInfo::FromJsonView(const JsonView & json)
{
	HardwareInfo hw;
	hw.cpuInfo = json.GetString("cpu").c_str();
	hw.gpuInfo = json.GetString("gpu").c_str();
	hw.ramInfo = json.GetString("ram").c_str();
	hw.systemName = json.GetString("system").c_str();

	return hw;
}

HardwareInfo HardwareInfo::Calculate()
{
	HardwareInfo hw;

	HRESULT hres;
	hres = CoInitializeEx(0, COINIT_MULTITHREADED);
	if (FAILED(hres))
		throw std::exception("Failed to initialize COM");

	/*hres = CoInitializeSecurity(
		NULL,
		-1,                          // COM authentication
		NULL,                        // Authentication services
		NULL,                        // Reserved
		RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication 
		RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation  
		NULL,                        // Authentication info
		EOAC_NONE,                   // Additional capabilities 
		NULL                         // Reserved
	);
	if (FAILED(hres))
		throw std::exception("Failed to initialize COM security");*/

	IWbemLocator *pLoc = NULL;

	hres = CoCreateInstance(
		CLSID_WbemLocator,
		0,
		CLSCTX_INPROC_SERVER,
		IID_IWbemLocator, (LPVOID *)&pLoc);
	if (FAILED(hres))
		throw std::exception("Failed to create COM instance");

	IWbemServices *pSvc = NULL;
	hres = pLoc->ConnectServer(
		_bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
		NULL,                    // User name. NULL = current user
		NULL,                    // User password. NULL = current
		0,                       // Locale. NULL indicates current
		NULL,                    // Security flags.
		0,                       // Authority (for example, Kerberos)
		0,                       // Context object 
		&pSvc                    // pointer to IWbemServices proxy
	);
	if (FAILED(hres))
		throw std::exception("Failed to connect to COM server");

	hres = CoSetProxyBlanket(
		pSvc,                        // Indicates the proxy to set
		RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
		RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
		NULL,                        // Server principal name 
		RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx 
		RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
		NULL,                        // client identity
		EOAC_NONE                    // proxy capabilities 
	);
	if (FAILED(hres))
		throw std::exception("Failed to set COM proxy blanket");


	hw.gpuInfo = GetWMIPropertyFrom(pSvc, L"PNPDeviceID", L"Win32_VideoController");
	hw.cpuInfo = GetWMIPropertyFrom(pSvc, L"ProcessorId", L"Win32_Processor");
	hw.ramInfo = GetWMIPropertyFrom(pSvc, L"SerialNumber", L"Win32_PhysicalMemory");
	hw.systemName = GetWMIPropertyFrom(pSvc, L"name", L"win32_computersystem");

	pSvc->Release();
	pLoc->Release();
	CoUninitialize();
	return hw;
}

JsonValue HardwareInfo::ToJsonValue() const
{
	JsonValue json;

	json.WithString("cpu", cpuInfo.c_str());
	json.WithString("gpu", gpuInfo.c_str());
	json.WithString("ram", ramInfo.c_str());
	json.WithString("system", systemName.c_str());

	return json;
}

std::string HardwareInfo::ToJsonString() const
{
	static char buff[2048];
	sprintf_s(buff, "{ \"cpu\" : \"%s\", \"gpu\": \"%s\", \"ram\": \"%s\", \"system\": \"%s\"}", cpuInfo.c_str(), gpuInfo.c_str(), ramInfo.c_str(), systemName.c_str());

	return std::string(buff);
}