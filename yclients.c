/**
 * File              : yclients.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 30.10.2021
 * Last Modified Date: 01.11.2021
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */
#include "yclients.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#ifdef __APPLE__
    #include <TargetConditionals.h>
#endif
#if defined TARGET_OS_IPHONE || defined TARGET_IPHONE_SIMULATOR
	#include "resources/curl/iOS/include/curl/curl.h"
#elif defined __ANDROID__
    #include "resources/curl/android/include/curl/curl.h"
#elif defined _WIN32
	#include "resources/curl/windows/win32/include/curl/curl.h"
#elif defined _WIN64
	#include "resources/curl/windows/win64/include/curl/curl.h"
#else
	#include <curl/curl.h>
#endif
#include <stdbool.h>
#include <pthread.h>

static char USER_TOKEN[64] = {0}; //user token - updates once

#pragma region <YCLIENTS LIB>
struct string {
	char *ptr;
	size_t len;
};

void init_string(struct string *s) {
	s->len = 0;
	s->ptr = malloc(s->len+1);
	if (s->ptr == NULL) {
		perror("string malloc");
		exit(EXIT_FAILURE);
	}
	s->ptr[0] = '\0';
}


size_t writefunc(void *ptr, size_t size, size_t nmemb, struct string *s)
{
	size_t new_len = s->len + size*nmemb;
	s->ptr = realloc(s->ptr, new_len+1);
	if (s->ptr == NULL) {
		perror("string realloc");
		exit(EXIT_FAILURE);
	}
	memcpy(s->ptr+s->len, ptr, size*nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;

	return size*nmemb;
}


char *yclients_get_user_token(const char partner_token[21], const char *login, const char *password){
	CURL *curl = curl_easy_init();
		
	struct string s;
	init_string(&s);

	if(curl) {
		char requestString[BUFSIZ];	
		sprintf(requestString, "%s/auth", URL);
		
		curl_easy_setopt(curl, CURLOPT_URL, requestString);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");		
		curl_easy_setopt(curl, CURLOPT_HEADER, false);

		char auth[128];
		sprintf(auth, "Authorization: Bearer %s", partner_token);

		struct curl_slist *header = NULL;
	    header = curl_slist_append(header, "Accept: application/vnd.yclients.v2+json");		
	    header = curl_slist_append(header, "Connection: close");		
	    header = curl_slist_append(header, "Content-Type: application/json");		
	    header = curl_slist_append(header, auth);		
		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

		char post[BUFSIZ];
		sprintf(post, "{\"login\":\"%s\",\"password\":\"%s\"}", login, password);		
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, strlen(post));
		
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); //do not verify sertificate		

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_slist_free_all(header);
		if (res) { //handle erros
            char *returned_code = malloc(BUFSIZ);
            sprintf(returned_code, "CURL ERROR: %d", res);
            return returned_code;			
		}		
	}

	//printf("USER_TOKEN: %s\n", s.ptr);

	cJSON *json = cJSON_Parse(s.ptr);
	free(s.ptr);
	if (cJSON_IsObject(json)) {
		cJSON *data = cJSON_GetObjectItem(json, "data");
		if (cJSON_IsObject(data)) {
			cJSON *user_token = cJSON_GetObjectItem(data, "user_token");			
			strncpy(USER_TOKEN, user_token->valuestring, 63);
			USER_TOKEN[63] = '\0';
			return user_token->valuestring;
		}
	}	
	return NULL;
}

char *curl_get_request(const char partner_token[21], const char *login, const char *password, const char *requestString){
	char *user_token;
	
	if (USER_TOKEN[0] == '\0') {
		user_token = yclients_get_user_token(partner_token, login, password);
	}
	else {
		user_token = USER_TOKEN;
	}
	
	struct string s;
	init_string(&s);

	CURL *curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, requestString);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");		
		curl_easy_setopt(curl, CURLOPT_HEADER, false);

		struct curl_slist *header = NULL;
	    header = curl_slist_append(header, "Accept: application/vnd.yclients.v2+json");		
	    header = curl_slist_append(header, "Connection: close");		
		char auth[128]; sprintf(auth, "Authorization: Bearer %s, User %s", partner_token, user_token);
	    header = curl_slist_append(header, auth);		
		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); //do not verify sertificate		

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_slist_free_all(header);	

		if (res) { //handle erros
            char *returned_code = malloc(BUFSIZ);
            sprintf(returned_code, "CURL ERROR: %d", res);
            return returned_code;			
		}				
	}
	return s.ptr;
}

char *curl_post_request(const char partner_token[21], const char *login, const char *password, const char *requestString, const char *postString){
	char *user_token;
	
	if (USER_TOKEN[0] == '\0') {
		user_token = yclients_get_user_token(partner_token, login, password);
	}
	else {
		user_token = USER_TOKEN;
	}
	
	struct string s;
	init_string(&s);

	CURL *curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, requestString);
		curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");		
		curl_easy_setopt(curl, CURLOPT_HEADER, false);

		struct curl_slist *header = NULL;
	    header = curl_slist_append(header, "Accept: application/vnd.yclients.v2+json");		
	    header = curl_slist_append(header, "Connection: close");		
	    header = curl_slist_append(header, "Content-Type: application/json");		
		char auth[128]; sprintf(auth, "Authorization: Bearer %s, User %s", partner_token, user_token);
	    header = curl_slist_append(header, auth);		
		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postString);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);

        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); //do not verify sertificate		

		CURLcode res = curl_easy_perform(curl);

		curl_easy_cleanup(curl);
		curl_slist_free_all(header);	

		if (res) { //handle erros
            char *returned_code = malloc(BUFSIZ);
            sprintf(returned_code, "CURL ERROR: %d", res);
            return returned_code;			
		}				
	}
	return s.ptr;
}

void json_to_client(cJSON *json, struct yclients_client_t *client){

	cJSON *id = cJSON_GetObjectItem(json, "id");	
	if (id != NULL) {	
		client->id = id->valueint;
	}	
	
	cJSON *name = cJSON_GetObjectItem(json, "name");	
	if (name != NULL) {	
		strncpy(client->name, name->valuestring, sizeof(client->name) - 1);
		client->name[sizeof(client->card) - 1] = '\0';
	}

	cJSON *phone = cJSON_GetObjectItem(json, "phone");	
	if (phone != NULL) {
		strncpy(client->phone, phone->valuestring, sizeof(client->phone) - 1);
		client->phone[sizeof(client->phone) - 1] = '\0';
	}	

	cJSON *card = cJSON_GetObjectItem(json, "card");	
	if (card != NULL) {
		strncpy(client->card, card->valuestring, sizeof(client->card) - 1);
		client->card[sizeof(client->card) - 1] = '\0';
	}

	cJSON *birth_date = cJSON_GetObjectItem(json, "birth_date");	
	if (birth_date != NULL) {	
		strncpy(client->birth_date, birth_date->valuestring, sizeof(client->birth_date) - 1);
		client->birth_date[sizeof(client->birth_date) - 1] = '\0';
	}
	
	cJSON *comment = cJSON_GetObjectItem(json, "comment");	
	if (comment != NULL) {		
		strncpy(client->comment, comment->valuestring, sizeof(client->comment) - 1);
		client->comment[sizeof(client->comment) - 1] = '\0';
	}
	
	cJSON *discount = cJSON_GetObjectItem(json, "discount");	
	if (discount != NULL) {			
		client->discount = discount->valueint;
	}
	
	cJSON *visits = cJSON_GetObjectItem(json, "visits");	
	if (visits != NULL) {				
		client->visits = visits->valueint;	
	}
	
	cJSON *sex_id = cJSON_GetObjectItem(json, "sex_id");	
	if (sex_id != NULL) {					
		client->sex_id = sex_id->valueint;	
	}
	
	cJSON *sex = cJSON_GetObjectItem(json, "sex");	
	if (sex != NULL) {						
		strncpy(client->sex, sex->valuestring, sizeof(client->sex) - 1);
		client->sex[sizeof(client->sex) - 1] = '\0';
	}
	
	cJSON *sms_check = cJSON_GetObjectItem(json, "sms_check");
	if (sms_check != NULL) {							
		client->sms_check = sms_check->valueint;
	}	
	
	cJSON *sms_bot = cJSON_GetObjectItem(json, "sms_bot");	
	if (sms_bot != NULL) {						
		client->sms_bot = sms_bot->valueint;
	}	
	
	cJSON *sms_not = cJSON_GetObjectItem(json, "sms_not");
	if (sms_not != NULL) {							
		client->sms_not = sms_not->valueint;
	}	
	
	cJSON *spent = cJSON_GetObjectItem(json, "spent");	
	if (spent != NULL) {								
		client->spent = spent->valueint;
	}	
	
	cJSON *paid = cJSON_GetObjectItem(json, "paid");	
	if (paid != NULL) {									
		client->paid = paid->valueint;
	}	
	
	cJSON *balance = cJSON_GetObjectItem(json, "balance");
	if (balance != NULL) {										
		client->balance = balance->valueint;
	}	
	
	cJSON *importance_id = cJSON_GetObjectItem(json, "importance_id");	
	if (importance_id != NULL) {											
		client->importance_id = importance_id->valueint;
	}	
	
	cJSON *importance = cJSON_GetObjectItem(json, "importance");
	if (importance != NULL) {												
		strncpy(client->importance, importance->valuestring, sizeof(client->importance) - 1);
		client->importance[sizeof(client->importance) - 1] = '\0';
	}
	
	cJSON *last_change_date = cJSON_GetObjectItem(json, "last_change_date");
	if (last_change_date != NULL) {													
		strncpy(client->last_change_date, last_change_date->valuestring, sizeof(client->last_change_date) - 1);
		client->last_change_date[sizeof(client->last_change_date) - 1] = '\0';
	}
}


void *get_all_clients(void *param) 
{
	struct thread_params *params = param;
	struct yclients_client_t *clients = params->data;

	int get_count = 0, total = 1, page = 1;
	
	bool hasFinished = false;
	
	while (get_count < total) {
		char requestString[BUFSIZ];	
		sprintf(requestString, "%s/clients/%ld?page=%d&count=%d", URL, params->companyId, page, 100);
					
		int count = 0;

		char *ret = curl_get_request(params->partner_token, params->login, params->password, requestString);
		//printf("CLIENTS: %s\n", ret);
		cJSON *json = cJSON_Parse(ret);
		free(ret);

		if (cJSON_IsObject(json)) {
			cJSON *success = cJSON_GetObjectItem(json, "success");
			if (!success->valueint) {
				break;
			}		
			cJSON *meta = cJSON_GetObjectItem(json, "meta");
			if (cJSON_IsObject(meta)) {
				cJSON *total_count = cJSON_GetObjectItem(meta, "total_count");
				total = total_count->valueint;
			}		
			cJSON *data = cJSON_GetObjectItem(json, "data");
			if (cJSON_IsArray(data)) {
				cJSON *item = data->child;			
				while (item != NULL) {

					struct yclients_client_t client;
					
					json_to_client(item, &client);
					
					clients[count] = client;
					count++;
					get_count++;
					
					item = item->next;				
				}
			}
		}
		page++;
		if (get_count == total) {
			hasFinished = true;
		}
		if (params->callback(count, clients, hasFinished)) {
			//stop function if callback returned not zero
			fprintf(stderr, "Function get_all_clients stoped as callback returned non zero\n");
			break;
		}
	}
	
	free(params);

	pthread_exit(0);
}

int yclients_get_all_clients(struct yclients_client_t **_clients, const char partner_token[21], const char *login, const char *password, long companyId, int (*callback)(int count, void *clients, bool hasFinished)){
	int err = 0; //error
		
	struct yclients_client_t *clients = malloc(100 * sizeof(struct yclients_client_t));
	if (clients == NULL) {
		perror("malloc clients");
		exit(EXIT_FAILURE);
	}				
	if (_clients) {
		*_clients = clients;
	}

	pthread_t tid; //идентификатор потока
	pthread_attr_t attr; //атрибуты потока

	//получаем дефолтные значения атрибутов
	err = pthread_attr_init(&attr);
	if (err) {
		perror("THREAD attributes");
		return err;
	}	
	
	//allocate Reachability_params
	struct thread_params *params=malloc(5*8 + sizeof(int));
	if (params == NULL) {
		perror("thread_params");
		exit(EXIT_FAILURE);
	}

	//set paramets to callback
	params->partner_token = partner_token;
	params->login = login;
	params->password = password;
	params->companyId = companyId;
	params->callback = callback;
	params->data = clients;


	//создаем новый поток
	err = pthread_create(&tid,&attr,get_all_clients,params);
	if (err) {
		perror("create THREAD");
		return err;
	}

	return 0;	
}

int yclients_get_clients_with_request( struct yclients_client_t **_clients, const char partner_token[21], const char *login, const char *password, long companyId, const char *requestString){
	int count = 0;
	struct yclients_client_t *clients = malloc(sizeof(struct yclients_client_t));
	if (clients == NULL) {
		perror("malloc clients");
		exit(EXIT_FAILURE);	
	}

	char *ret = curl_get_request(partner_token, login, password, requestString);
	//printf("CLIENTS: %s\n", ret);
	cJSON *json = cJSON_Parse(ret);
	free(ret);
	if (cJSON_IsObject(json)) {
		cJSON *success = cJSON_GetObjectItem(json, "success");
		if (!success->valueint) {
			return 0;
		}		
		cJSON *meta = cJSON_GetObjectItem(json, "meta");
		if (cJSON_IsObject(meta)) {
			cJSON *total_count = cJSON_GetObjectItem(meta, "total_count");
			clients = realloc(clients, (total_count->valueint + 1) * sizeof(struct yclients_client_t));
			if (clients == NULL) {
				perror("realloc clients");
				exit(EXIT_FAILURE);
			}				
		}		
		cJSON *data = cJSON_GetObjectItem(json, "data");
		if (cJSON_IsArray(data)) {
			cJSON *item = data->child;			
			while (item != NULL) {

				struct yclients_client_t client;
				
				json_to_client(item, &client);
				
				clients[count] = client;
				count++;
				
				item = item->next;				
			}
		}
	}
	
	if (_clients) {
		*_clients = clients;
	}

	return count;	
	
}

int yclients_get_clients_with_search_phone( struct yclients_client_t **clients, const char partner_token[21], const char *login, const char *password, long companyId, long phone){
	char requestString[BUFSIZ];	
	sprintf(requestString, "%s/clients/%ld?phone=%ld&page=%d&count=%d", URL, companyId, phone, 1, 60);	
	return yclients_get_clients_with_request(clients, partner_token, login, password, companyId, requestString);
}

int yclients_get_clients_with_search_name( struct yclients_client_t **clients, const char partner_token[21], const char *login, const char *password, long companyId, const char *name){
	char requestString[BUFSIZ];	
	sprintf(requestString, "%s/clients/%ld?fullname=%s&page=%d&count=%d", URL, companyId, name, 1, 60);	
	return yclients_get_clients_with_request(clients, partner_token, login, password, companyId, requestString);	
}

int yclients_get_clients_with_search_pattern( struct yclients_client_t **clients, const char partner_token[21], const char *login, const char *password, long companyId, const char *searchPattern){
	long phone;
	if (sscanf(searchPattern, "%ld", &phone) == 1){
		printf("SEARCH PHONE: %ld\n", phone);
		if (searchPattern[0] == '8') {
			//change first number "8" to "7"
			char *new_pattern = malloc(strlen(searchPattern) * sizeof(char) + 1);
			strcpy(new_pattern, searchPattern);
			new_pattern[0] = '7';
			sscanf(new_pattern, "%ld", &phone);
			free(new_pattern);
		}
		//searching phone
		return yclients_get_clients_with_search_phone(clients, partner_token, login, password, companyId, phone);
	}
	else {
		//search name
		return yclients_get_clients_with_search_name(clients, partner_token, login, password, companyId, searchPattern);
	}

	return 0;
}

struct yclients_client_t yclients_get_client (const char partner_token[21], const char *login, const char *password, long companyId, long clientId){
	struct yclients_client_t client;
	char requestString[BUFSIZ];	
	sprintf(requestString, "%s/client/%ld/%ld", URL, companyId, clientId);	
	char *ret = curl_get_request(partner_token, login, password, requestString);	

	cJSON *json = cJSON_Parse(ret);
	free(ret);
	if (cJSON_IsObject(json)) {
		cJSON *data = cJSON_GetObjectItem(json, "data");
		if (cJSON_IsObject(data)) {
				json_to_client(data, &client);
		}
	}
	return client;
}

int yclients_get_clients_with_search( struct yclients_client_t **_clients, const char partner_token[21], const char *login, const char *password, long companyId,char **fields, int fields_count, const char *searchString){

	struct yclients_client_t client;
	char requestString[BUFSIZ];	
	sprintf(requestString, "%s/company/%ld/clients/search", URL, companyId);	

	char searchFields[BUFSIZ];
	sprintf(searchFields, "");
	
	char *field = fields[0];
	sprintf(searchFields, "%s\"%s\"", searchFields, field);
	int i;
	for (i = 1; i < fields_count; ++i) {
		field = fields[i];	
		sprintf(searchFields, "%s, \"%s\"", searchFields, field);
	}

	char postString[BUFSIZ];
	sprintf(postString, "{\"page\": 1,\"page_size\": 100, \"fields\": [%s], \"order_by\": \"name\", \"order_by_direction\": \"asc\", \"operation\": \"AND\", \"filters\": [{\"type\": \"quick_search\", \"state\": {\"value\": \"%s\"}}]}", searchFields, searchString);

	int count = 0;
	struct yclients_client_t *clients = malloc(sizeof(struct yclients_client_t));
	if (clients == NULL) {
		perror("malloc clients");
		exit(EXIT_FAILURE);	
	}	

	char *ret = curl_post_request(partner_token, login, password, requestString, postString);	

	cJSON *json = cJSON_Parse(ret);
	free(ret);
	if (cJSON_IsObject(json)) {
		cJSON *success = cJSON_GetObjectItem(json, "success");
		if (!success->valueint) {
			return 0;
		}		
		cJSON *meta = cJSON_GetObjectItem(json, "meta");
		if (cJSON_IsObject(meta)) {
			cJSON *total_count = cJSON_GetObjectItem(meta, "total_count");
			clients = realloc(clients, (total_count->valueint + 1) * sizeof(struct yclients_client_t));
			if (clients == NULL) {
				perror("realloc clients");
				exit(EXIT_FAILURE);
			}				
		}		
		cJSON *data = cJSON_GetObjectItem(json, "data");
		if (cJSON_IsArray(data)) {
			cJSON *item = data->child;			
			while (item != NULL) {

				struct yclients_client_t client;
				
				json_to_client(item, &client);
				
				clients[count] = client;
				count++;
				
				item = item->next;				
			}
		}
	}
	
	if (_clients) {
		*_clients = clients;
	}

	return count;	
}

//time_t time_tFromDatetime(char *datetime, struct tm *timestruct){
	//struct tm tm;
	////strptime(datetime, "'%Y-%m-%dT%H:%M:%S%z'", &tm);
	//strptime(datetime, "'%Y-%m-%dT%H:%M:%S.%f%z'", &tm);

	//if (timestruct != NULL) {
		//*timestruct = tm;	
	//}
	//return mktime(&tm);
//}

void timeToRUS(const char *ISOTime, char *RUSTime){
	char year[4];
	char mounth[2];
	char day[2];
	sscanf(ISOTime, "%c%c%c%c-%c%c-%c%cT", &year[0],&year[1],&year[2],&year[3], &mounth[0],&mounth[1], &day[0],&day[1]);
	sprintf(RUSTime, "%c%c.%c%c.%c%c%c%c", day[0], day[1], mounth[0], mounth[1], year[0], year[1], year[2], year[3]);
}

void json_to_event(cJSON *json, struct yclients_event_t *event){

	cJSON *id = cJSON_GetObjectItem(json, "id");	
	event->id = id->valueint;	
	
	cJSON *company_id = cJSON_GetObjectItem(json, "company_id");	
	event->company_id = company_id->valueint;	
	
	cJSON *staff_id = cJSON_GetObjectItem(json, "staff_id");	
	event->staff_id = staff_id->valueint;	
	
	//services
	//goods_transactions
	//staff
	
	cJSON *client = cJSON_GetObjectItem(json, "client");	
	json_to_client(client, &event->client);

	/*cJSON *comer = cJSON_GetObjectItem(json, "comer");	*/
	/*strncpy(event->comer, comer->valuestring, sizeof(event->comer) - 1);*/
	/*event->comer[sizeof(event->comer) - 1] = '\0';*/
	
	cJSON *clients_count = cJSON_GetObjectItem(json, "clients_count");	
	event->clients_count = staff_id->valueint;	
	
	cJSON *date = cJSON_GetObjectItem(json, "date");	
	timeToRUS(date->valuestring, event->date);
	
	char *datetime = cJSON_GetObjectItem(json, "datetime")->valuestring;
	strncpy(event->datetime, datetime, sizeof(event->datetime) - 1);
	event->datetime[sizeof(event->datetime) - 1] = '\0';

	cJSON *create_date = cJSON_GetObjectItem(json, "create_date");	
	strncpy(event->create_date, create_date->valuestring, sizeof(event->create_date) - 1);
	event->create_date[sizeof(event->create_date) - 1] = '\0';
	
	cJSON *comment = cJSON_GetObjectItem(json, "comment");	
	strncpy(event->comment, comment->valuestring, sizeof(event->comment) - 1);
	event->comment[sizeof(event->comment) - 1] = '\0';
	
	cJSON *online = cJSON_GetObjectItem(json, "online");	
	event->online = online->valueint;	
	
	cJSON *visit_attendance = cJSON_GetObjectItem(json, "visit_attendance");	
	event->visit_attendance = visit_attendance->valueint;	
	
	cJSON *attendance = cJSON_GetObjectItem(json, "attendance");	
	event->attendance = attendance->valueint;	
	
	cJSON *confirmed = cJSON_GetObjectItem(json, "confirmed");	
	event->confirmed = confirmed->valueint;	
	
	cJSON *seance_length = cJSON_GetObjectItem(json, "seance_length");	
	event->seance_length = seance_length->valueint;	
	
	cJSON *length = cJSON_GetObjectItem(json, "length");	
	event->length = length->valueint;	
	
	cJSON *sms_before = cJSON_GetObjectItem(json, "sms_before");	
	event->sms_before = sms_before->valueint;	
	
	cJSON *sms_now = cJSON_GetObjectItem(json, "sms_now");	
	event->sms_now = sms_now->valueint;	
	
	cJSON *sms_now_text = cJSON_GetObjectItem(json, "sms_now_text");	
	strncpy(event->sms_now_text, sms_now_text->valuestring, sizeof(event->sms_now_text) - 1);
	event->sms_now_text[sizeof(event->sms_now_text) - 1] = '\0';
	
	cJSON *email_now = cJSON_GetObjectItem(json, "email_now");	
	event->email_now = email_now->valueint;	
	
	cJSON *notified = cJSON_GetObjectItem(json, "notified");	
	event->notified = notified->valueint;	
	
	cJSON *master_request = cJSON_GetObjectItem(json, "master_request");	
	event->master_request = master_request->valueint;	
	
	cJSON *api_id = cJSON_GetObjectItem(json, "api_id");	
	strncpy(event->api_id, api_id->valuestring, sizeof(event->api_id) - 1);
	event->api_id[sizeof(event->api_id) - 1] = '\0';
	
	cJSON *from_url = cJSON_GetObjectItem(json, "from_url");	
	strncpy(event->from_url, from_url->valuestring, sizeof(event->from_url) - 1);
	event->from_url[sizeof(event->from_url) - 1] = '\0';
	
	cJSON *review_requested = cJSON_GetObjectItem(json, "review_requested");	
	event->review_requested = review_requested->valueint;	
	
	cJSON *visit_id = cJSON_GetObjectItem(json, "visit_id");	
	event->visit_id = visit_id->valueint;	
	
	cJSON *created_user_id = cJSON_GetObjectItem(json, "created_user_id");	
	event->created_user_id = created_user_id->valueint;	
	
	cJSON *deleted = cJSON_GetObjectItem(json, "deleted");	
	event->deleted = deleted->valueint;	
	
	cJSON *paid_full = cJSON_GetObjectItem(json, "paid_full");	
	event->paid_full = paid_full->valueint;	
	
	cJSON *payment_status = cJSON_GetObjectItem(json, "payment_status");	
	event->payment_status = payment_status->valueint;	
	
	cJSON *prepaid = cJSON_GetObjectItem(json, "prepaid");	
	event->prepaid = prepaid->valueint;	
	
	cJSON *prepaid_confirmed = cJSON_GetObjectItem(json, "prepaid_confirmed");	
	event->prepaid_confirmed = prepaid_confirmed->valueint;	
	
	cJSON *last_change_date = cJSON_GetObjectItem(json, "last_change_date");	
	strncpy(event->last_change_date, last_change_date->valuestring, sizeof(event->last_change_date) - 1);
	event->last_change_date[sizeof(event->last_change_date) - 1] = '\0';
	
	cJSON *custom_color = cJSON_GetObjectItem(json, "custom_color");	
	strncpy(event->custom_color, custom_color->valuestring, sizeof(event->custom_color) - 1);
	event->custom_color[sizeof(event->custom_color) - 1] = '\0';
	
	cJSON *custom_font_color = cJSON_GetObjectItem(json, "custom_font_color");	
	strncpy(event->custom_font_color, custom_font_color->valuestring, sizeof(event->custom_font_color) - 1);
	event->custom_font_color[sizeof(event->custom_font_color) - 1] = '\0';
	
	//record_labels
	
	cJSON *activity_id = cJSON_GetObjectItem(json, "activity_id");	
	event->activity_id = activity_id->valueint;	
	
	//custom_fields;
	//documents
	
	cJSON *sms_remain_hours = cJSON_GetObjectItem(json, "sms_remain_hours");	
	event->sms_remain_hours = sms_remain_hours->valueint;	
	
	cJSON *email_remain_hours = cJSON_GetObjectItem(json, "email_remain_hours");	
	event->email_remain_hours = email_remain_hours->valueint;	
	
	cJSON *bookform_id = cJSON_GetObjectItem(json, "bookform_id");	
	event->bookform_id = bookform_id->valueint;	
	
	cJSON *record_from = cJSON_GetObjectItem(json, "record_from");	
	strncpy(event->record_from, record_from->valuestring, sizeof(event->record_from) - 1);
	event->record_from[sizeof(event->record_from) - 1] = '\0';
	
	cJSON *is_mobile = cJSON_GetObjectItem(json, "is_mobile");	
	event->is_mobile = is_mobile->valueint;	
	
	cJSON *is_sale_bill_printed = cJSON_GetObjectItem(json, "is_sale_bill_printed");	
	event->is_sale_bill_printed = is_sale_bill_printed->valueint;	
}


int yclients_get_events_for_client(struct yclients_event_t **_events, const char partner_token[21], const char *login, const char *password, long companyId, long clientId){
	int count = 0;	
	struct yclients_event_t *events = malloc(sizeof(struct yclients_event_t));
	if (events == NULL) {
		perror("malloc events");
		exit(ENOMEM);	
	}

	char requestString[BUFSIZ];		
	sprintf(requestString, "%s/records/%ld?client_id=%ld", URL, companyId, clientId);
	
	char *ret = curl_get_request(partner_token, login, password, requestString);
	cJSON *json = cJSON_Parse(curl_get_request(partner_token, login, password, requestString));
	free(ret);

	if (cJSON_IsObject(json)) {
		cJSON *success = cJSON_GetObjectItem(json, "success");
		if (!success->valueint) {
			return 0;
		}
		cJSON *meta = cJSON_GetObjectItem(json, "meta");
		if (cJSON_IsObject(meta)) {
			cJSON *total_count = cJSON_GetObjectItem(meta, "total_count");
			count = total_count->valueint; 
			events = realloc(events, count * sizeof(struct yclients_event_t));
			if (events == NULL) {
				perror("realloc events");
				exit(ENOMEM);
			}			
		}		
		cJSON *data = cJSON_GetObjectItem(json, "data");
		printf("EVENTS: %s\n", cJSON_Print(data));
		if (cJSON_IsArray(data)) {
			int i;
			cJSON *item = data->child;			
			for (i = 0; i < count; ++i) {
				struct yclients_event_t event;
				json_to_event(item, &event);
				events[i] = event;
				item = item->next;				
			}
		}
	}	

	if (_events) {
		*_events = events;
	}	

	return count;
}
#pragma endregion <YCLIENTS LIB>
