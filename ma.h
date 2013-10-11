#ifndef MA_H
#define MA_H

#include <stdio.h>
#include <stdint.h>

#include <curl/curl.h>

#include "stream.h"

#define MAX_URL		512
#define MAX_HDR		512
#define MAX_NHDRS	64
#define MAX_CONTENT	1000000 /* 1MB of storage, use carefully */

#define MA_OK		0
#define MA_EINVAL	-1
#define MA_EMALFORMED	-2
#define MA_ENOTFOUND	-3
#define MA_ECONNECTION	-4

#define MA_SVC_MEDIUS	"MediusService.svc"
#define MA_SVC_LOG	"LogService.svc"
#define MA_SVC_DATA	"DataService.svc"
#define MA_SVC_UTILS	"UtilsService.svc"
#define MA_SVC_NAV	"NavigationService.svc"
#define MA_SVC_FILTER	"FilterService.svc"
#define MA_SVC_LOGIN	"LoginService.svc"
#define MA_SVC_VANDAAG	"VandaagService.svc"
#define MA_SVC_AGENDA	"AgendaService.svc"
#define MA_SVC_ROOSTER	"RoosterwijzigingenService.svc"

#define MA_SOAP_PREFIX	"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\"><s:Body><data>"
#define MA_SOAP_POSTFIX	"</data></s:Body></s:Envelope>"

#define MA_ZIP_NAME	"content"
#define MA_ZIP_PASSWORD	"yawUBRu+reduka5UPha2#=cRUc@ThekawEvuju&?g$dru9ped=a@REQ!7h_?anut"
#define MA_ZIP_MODTIME	0x5eed

struct ma_medius
{
	CURL* curl;

	char url_base[MAX_URL];
	char url_v[MAX_URL];
};

int ma_medius_init (struct ma_medius* m, const char* name);
int ma_request_init_data (struct ma_medius *m); /* TODO: TEMP: DELETE (or rename, probably internal) */

#endif