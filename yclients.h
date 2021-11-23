/**
 * File              : yclients.h
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 30.10.2021
 * Last Modified Date: 01.11.2021
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#define URL "https://api.yclients.com/api/v1"
#define PORT 443

#ifdef __cplusplus
extern "C"{
#endif 

//struct to pass arguments from thread to function
struct thread_params{
	const char *partner_token;
	const char *login;
	const char *password;
	long companyId;
	void *data;
	int (*callback)(int, void*, bool);
};
	

struct yclients_client_t{
	int id;
	char name[256];
	char phone[128];
	char email[128];
	char card[256];
	char birth_date[32];
	char comment[256];
	int discount;
	int visits;
	int sex_id;
	char sex[32];
	int sms_check;
	int sms_bot;
	int sms_not;
	int spent;
	int paid;
	int balance;
	int importance_id;
	char importance[64];
	//categories;
	char last_change_date[32];
	//custom_fields;
};

struct yclients_event_t{
	int id;
	int company_id;
	int staff_id;
	//services
	//goods_transactions
	//staff
	struct yclients_client_t client;
	//char comer[128];
	int clients_count;
	char date[32];
	char datetime[32];
	char create_date[32];
	char comment[256];
	bool online;
	int visit_attendance;
	int attendance;
	int confirmed;
	int seance_length;
	int length;
	int sms_before;
	int sms_now;
	char sms_now_text[BUFSIZ];
	int email_now;
	int notified;
	int master_request;
	char api_id[32];
	char from_url[128];
	int review_requested;
	int visit_id;
	int created_user_id;
	bool deleted;
	int paid_full;
	int payment_status;
	bool prepaid;
	bool prepaid_confirmed;
	char last_change_date[32];
	char custom_color[32];
	char custom_font_color[32];
	//record_labels
	int activity_id;
	//custom_fields;
	//documents
	int sms_remain_hours;
	int email_remain_hours;
	int bookform_id;
	char record_from[128];
	int is_mobile;
	bool is_sale_bill_printed;
};

//return string with user token for yclients for user with password
char *yclients_get_user_token(const char partner_token[21], const char *login, const char *password);

//Get all clients from yclients company - asynch request.
int yclients_get_all_clients 
(
				struct yclients_client_t **clients, //array of clients
				const char partner_token[21],		//partner_token - YCLIENTS give it for developers
				const char *login,					//login - usualy telephone number in format (79991234567) or email
				const char *password,				//password
				long companyId,						//ID of company
				int (*callback)(                    //callback - return non 0 to stop function
					int count,						//count of array
					void *clients,					//array of clients (struct yclients_client_t)
					bool hasFinished				//true if has got all records
				)
);

//Get clients with search pattern from yclients company. old API
int yclients_get_clients_with_search_pattern 
(
				struct yclients_client_t **clients, //array of clients
				const char partner_token[21],		//partner_token - YCLIENTS give it for developers
				const char *login,					//login - usualy telephone number in format (79991234567) or email
				const char *password,				//password
				long companyId,						//ID of company
				const char *searchPattern			//telephone number or name
);

//Get clients with search pattern from yclients company. Use POST API.
int yclients_get_clients_with_search
( 
				struct yclients_client_t **clients, //array of clients 
				const char partner_token[21],		//partner_token - YCLIENTS give it for developers 
				const char *login,					//login - usualy telephone number in format (79991234567) or email
				const char *password,				//password 
				long companyId,						//ID of company 
				char **fields,						//fields to return in *char[] - eg. char *fields[] = {"id", "name"} 
				int fields_count,					//count of fields to return 
				const char *searchString			//search string - eg. telephone number or name
);

//Get clients with search pattern from yclients company. Use POST API.
int yclients_get_clients_names_with_search
( 
				const char partner_token[21],		  //partner_token - YCLIENTS give it for developers 
				const char *login,					  //login - usualy telephone number in format (79991234567) or email
				const char *password,				  //password 
				long companyId,						  //ID of company 
				const char *searchString,			  //search string - eg. telephone number or name
				void context,						  //context to pass trow callback
				int (*callback)(                      //callback - return non 0 to stop function
					int error,						  //0 - no errors
					struct yclients_client_t *client, //pointer of client (struct yclients_client_t)
					void context				      //context callback return
				)
				
);


//Get client with clientId from yclients company.
struct yclients_client_t yclients_get_client 
(
				const char partner_token[21],		//partner_token - YCLIENTS give it for developers
				const char *login,					//login - usualy telephone number in format (79991234567) or email
				const char *password,				//password
				long companyId,						//ID of company
				long clientId					    //ID of client
);

//get all events (visits) for clientId
int yclients_get_events_for_client
(
				 struct yclients_event_t **events, //array of events
				 const char partner_token[21],     //partner token 
				 const char *login,				   //login 
				 const char *password,			   //password 
				 long companyId,					   //ID of company
				 long clientId					   //ID of client
);

#ifdef __cplusplus
}
#endif

