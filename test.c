/**
 * File              : test.c
 * Author            : Igor V. Sementsov <ig.kuzm@gmail.com>
 * Date              : 30.10.2021
 * Last Modified Date: 01.11.2021
 * Last Modified By  : Igor V. Sementsov <ig.kuzm@gmail.com>
 */

#include <stdio.h>
#include "yclients.h"


//int callback(int count, void *ptr, bool hasFinished){
	//struct yclients_client_t *clients = ptr;
	//printf("LOADED %d CLIENTS\n", count);
	//int i;
	//for (i = 0; i < count; ++i) {
		//printf("CLIENT ID: %d\tNAME: %s\n", clients[i].id, clients[i].name);
	//}
	//if (hasFinished) {
		//printf("FINISHED\n");
	//}
	//return 0;
//}

//6672348

int callback(int error, int count, struct yclients_client_t *clients, void *context){
	int i;
	for (i = 0; i < count; ++i) {
		printf("NAME: %s\n", clients[i].name);
	}
	return 0;
}

int main(int argc, char *argv[])
{
	//get_ueser_token("xr693jct2mdw7e9dzzge", "79990407731", "N0LamerSplZ");
	
	//struct yclients_client_t *clients;
	//int count = yclients_get_all_clients(&clients, "xr693jct2mdw7e9dzzge", "79147341055", "puewj3", 69643, callback);
	//printf("HELLO WORLD\n");
	//getchar();
	/*printf("GET %d CLIENTS\n", count);*/
	/*int i;*/
	/*for (i = 0; i < count; ++i) {*/
		/*printf("CLIENT ID: %d, NAME: %s\n", clients[i].id, clients[i].name);*/
		
	/*}*/

	//ID 120372693
	//
	//struct yclients_event_t *events;
	//int count = yclients_get_events_for_client(&events, "xr693jct2mdw7e9dzzge", "79147341055", "puewj3", 69643, 35390745);	
	//int i;
	//for (i = 0; i < count; ++i) {
		//struct yclients_client_t client = events[i].client;
		//printf("EVENT ID: %d, DATE: %s, CLIENT: %s\n", events[i].id, events[i].date, client.name);
	//}
	//
	//struct yclients_client_t *clients;
	//int count = yclients_get_clients_with_search_pattern(&clients, "xr693jct2mdw7e9dzzge", "79147341055", "puewj3", 69643, "89990407731");
	//printf("GET %d CLIENTS\n", count);
	//int i;
	//for (i = 0; i < count; ++i) {
		//printf("ID: %d, NAME: %s, PHONE: %s\n", clients[i].id, clients[i].name, clients[i].phone);
		
	//}
	
	yclients_get_clients_names_with_search("xr693jct2mdw7e9dzzge", "79147341055", "puewj3", 69643, "8999040", NULL, callback);
	
	getchar();
	
	/*struct yclients_client_t *clients;*/
	/*char *fields[] = {"id", "name", "phone"};*/
	/*int count = yclients_get_clients_with_search(&clients, "xr693jct2mdw7e9dzzge", "79147341055", "puewj3", 69643, fields, 3,  "89990407731");*/
	/*printf("GET %d CLIENTS\n", count);*/
	/*int i;*/
	/*for (i = 0; i < count; ++i) {*/
		/*printf("ID: %d, NAME: %s, PHONE: %s\n", clients[i].id, clients[i].name, clients[i].phone);*/
		
	/*}	*/



	//35390745

	//struct yclients_client_t client = yclients_get_client("xr693jct2mdw7e9dzzge", "79147341055", "puewj3", 69643, 35390745);
	//printf("ID: %d, NAME: %s, PHONE: %s\n", client.id, client.name, client.phone);

	return 0;
}
