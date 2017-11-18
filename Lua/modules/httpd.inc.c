/*
 * Copyright bhgv 2017
 */

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include <espressif/esp_common.h>
//#include <etstimer.h>

//typedef uint8_t uint8;

//#include <dhcpserver.h>

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "espressif/esp_common.h"
//#include "espressif/phy_info.h"

#include "lwip/api.h"
//#include "ipv4/lwip/ip.h"
#include "lwip/tcp.h"
//#include "lwip/tcp_impl.h"

#include "mbedtls/sha1.h"
#include "mbedtls/base64.h"

/*
#include <spiffs.h>
//#include <spiffs_nucleus.h>
#include <esp_spiffs.h>
*/

#include "httpd/httpd.h"




#if 0
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif



//extern spiffs fs;

int is_httpd_run = 0;


char *hdr, *hdr_sz;



#if 1 //LWIP_HTTPD_STRNSTR_PRIVATE
/** Like strstr but does not need 'buffer' to be NULL-terminated */

void system_soft_wdt_feed(){
//	wdt_flg = false;
//	WDT.FEED = WDT_FEED_MAGIC;
	sdk_wdt_feed();
}

char*
strnstr(const char* buffer, const char* token, size_t n)
{
  const char* p;
  int tokenlen = (int)strlen(token);
  if (tokenlen == 0) {
    return (char *)buffer;
  }
  if (n == 0) {
    n = (int)strlen(buffer);
  }
//DBG("strnstr 1 %s %s %d\n", buffer, token, n);
  for (p = buffer; *p && (p + tokenlen <= buffer + n); p++) {
    if ((*p == *token) && (strncmp(p, token, tokenlen) == 0)) {
//DBG("strnstr 2 %s\n", p);
      return (char *)p;
    }
  }
  return NULL;
}

char*
strstr(const char* buffer, const char* token)
{
  return strnstr(buffer, token, 0);
}

#endif /* LWIP_HTTPD_STRNSTR_PRIVATE */



#if 1

void nb_free();


const char webpage_404[] = {
        "<html><head><title>luos9 v0.1 Server</title>"
        "<style> div.main {"
        "font-family: Arial;"
        "padding: 0.01em 16px;"
        "box-shadow: 2px 2px 1px 1px #d2d2d2;"
        "background-color: #f1f1f1;}"
        "</style></head>"
        "<body><div class='main'>"
        "<h3>404</h3>"
        "<p>URL: %s</p>"
        "<p>Uptime: %d seconds</p>"
        "<p>Free heap: %d bytes</p>"
        "</div></body></html>"
};


void prep_something( ){

}


struct netbuf *nb=NULL;

void nb_free(){
	if(nb != NULL){
		netbuf_delete(nb);
		nb = NULL;
	}
}

void nc_free(struct netconn **nc, char* msg){
	if(*nc != NULL){
		if(msg != NULL) 
			printf(msg);
		netconn_close(*nc);
		netconn_delete(*nc);
		*nc = NULL;
	}
}



int recv_timeout = DEF_RECV_TIMEOUT;
int send_timeout = DEF_SEND_TIMEOUT;

struct netconn *client = NULL;
struct netconn *nc = NULL; //netconn_new(NETCONN_TCP);


int do_something(char **uri, int uri_len, char *hdr, char* hdr_sz, char *out ){

//	if (!strncmp(uri, "/on", uri_len))
//		gpio_write(2, false);
//	else if (!strncmp(uri, "/off", uri_len))
//		gpio_write(2, true);

//	out = NULL;
	return 0;
}


int do_404(char **uri, int uri_len, char *hdr, char* hdr_sz, /*char* data, int len,*/ char *out ){
	char* buf=NULL;
	int buf_len = 0;
	err_t err = ERR_OK;
	
	buf_len = sizeof(webpage_404) +20 + 1; //OUT_BUF_LEN;
	buf = malloc(buf_len); // OUT_BUF_LEN+1);

	if(buf){	
		//DBG("pre sprintf %x %d\n", buf, buf_len);
		snprintf(buf, buf_len, webpage_404,
				*uri,
				xTaskGetTickCount() * portTICK_PERIOD_MS / 1000,
				(int) xPortGetFreeHeapSize()
		);
		
		//free(uri);
		//uri_len = 0;
				
		//DBG("pre hdr_net_wrt %x %d\n", html_hdr, strlen(html_hdr) );
		usleep(10);
		if( check_conn(__func__, __LINE__) ){
			err = netconn_write(client, hdr, strlen(hdr), NETCONN_NOCOPY);
			print_err(err, __func__, __LINE__);
		}
		//if(err != ERR_OK)
		if(is_httpd_run == 4) 
		{
			free(buf);
			return 0;
		}
		
		if( check_conn(__func__, __LINE__) ){
			//char *hb = malloc(sizeof(hdr_sz)+10);
			//snprintf(hb, sizeof(hdr_siz)+10-1, hdr_sz, strlen(buf));
			//usleep(10);
//			err = netconn_write(client, hb, strlen(hb), NETCONN_NOCOPY);
			err = netconn_write(client, hdr_sz, strlen(hdr_sz), NETCONN_NOCOPY);
			//free(hb);
			
			print_err(err, __func__, __LINE__);
		}
		//if(err != ERR_OK)
		if(is_httpd_run == 4) 
		{
			free(buf);
			return 0;
		}
		
//		usleep(50);
		
		//DBG("pre net out %x %d\n%s\n", buf, buf_len, buf);
		if( check_conn(__func__, __LINE__) ){
			err = netconn_write(client, buf, buf_len, NETCONN_NOCOPY);
			
			free(buf);
			
			print_err(err, __func__, __LINE__);
		}else{
			free(buf);
		}
		//if(err != ERR_OK) 
		if(is_httpd_run == 4) 
			return 0;
//		usleep(50);
	//	buf_len = 0;
	}

	//*out = NULL;
	return buf_len;
}


int httpd_task(lua_State* L) //void *pvParameters)
{
err_t err = ERR_OK;
//	system_soft_wdt_feed();
	//taskYIELD();

//DBG( "_REENT = %x, __getreent = %x, _impure=%x\n\n", _REENT, __getreent(), _impure_ptr );
//	luaC_fullgc(L, 1);
//	static TaskHandle_t xHandleWs = NULL;
	//DBG("httpd_task 1 Free mem: %d\n",xPortGetFreeHeapSize());
//	DBG("MEMP_NUM_TCP_PCB = %d\n", MEMP_NUM_TCP_PCB);

	//lua_register(L, "GET_cnt", lget_get_params_cnt);
	lua_register(L, "_GET", lget_param);

//do{
	nc_free(&client, NULL);
//	nc_free(&nc, NULL);
//    /*struct netconn * */client = NULL;
	if( check_conn(__func__, __LINE__) ){
		/*struct netconn * */nc = netconn_new(NETCONN_TCP);
		printf("Open connection (main nc)\n");
	}else{
		nc_free(&nc, NULL);
	}

    if (nc == NULL || is_httpd_run == 4) {
        printf("Failed to allocate socket.\n");
        //vTaskDelete(NULL);
        return 0;
    }
	
	nc->recv_timeout = recv_timeout;
	nc->recv_bufsize = IN_BUF_LEN;

	ip_set_option(nc->pcb.tcp, SOF_REUSEADDR);
	//nc->pcb.tcp->so_options |= SOF_REUSEADDR;

	if( check_conn(__func__, __LINE__) ){
		err = netconn_bind(nc, IP_ADDR_ANY, 80);
		print_err(err, __func__, __LINE__);
	}

	if(is_httpd_run != 4){ 
		if( check_conn(__func__, __LINE__) ){
			err = netconn_listen(nc);
			print_err(err, __func__, __LINE__);

			prep_something();
		}
	}

	//is_httpd_run = 1;    
    while (is_httpd_run == 1 || is_httpd_run == 2) {
//		usleep(10);
//		taskYIELD();
//		system_soft_wdt_feed();
	
		//printf("httpd_task: Free mem: %d\n",xPortGetFreeHeapSize());
		//DBG("httpd_task iter: \nws_client=%x, ws_uri=%x, \nnc=%x, client=%x\nnb=%x, get_root=%x\n",
		//	ws_client, ws_uri, nc, client, nb, get_root
		//);

		usleep(10);
		nc_free(&client, NULL);
		if( check_conn(__func__, __LINE__) ){
			err = netconn_accept(nc, &client);
			print_err(err, __func__, __LINE__);
		}
		//if(err != ERR_OK) 
		if(is_httpd_run == 4) 
			break;
			
        if (client != NULL) 
			printf("Open connection (client) %x\n", client);
//		usleep(50);
        if (client != NULL && err == ERR_OK) {
            //struct netbuf *nb=NULL;
            //nb_free();
			client->send_timeout = send_timeout;
			client->recv_timeout = recv_timeout;
			client->recv_bufsize = IN_BUF_LEN;

			ip_set_option(client->pcb.tcp, SOF_REUSEADDR);
			//client->pcb.tcp->so_options |= SOF_REUSEADDR;
			
			//client->recv_bufsize = IN_BUF_LEN;
			//usleep(10);
			if( 
				check_conn(__func__, __LINE__) &&
				(err = print_err( netconn_recv(client, &nb), __func__, __LINE__ ) ) == ERR_OK
			) {
                void *data;
                u16_t len;
			
				if( !(is_httpd_run == 1 || is_httpd_run == 2) ){
					/*
					nb_free();
					nc_free(&client, "Closing connection (client) on http exit\n");
					ws_sock_del();
					if(get_root != NULL){
						get_list_free(&get_root);
					}
					//usleep(50);
					*/
					break;
				}
				//usleep(50);

				if( check_conn(__func__, __LINE__) ){
	                err = netbuf_data(nb, &data, &len);
					print_err(err, __func__, __LINE__);
				}
				//if(err != ERR_OK) 
				if(is_httpd_run == 4) 
					break;
				//DBG("%s\n", data);
				
                /* check for a GET request */
                if (!strncmp(data, "GET ", 4)) {
                    char *uri = NULL;
					char *suf = NULL;
					int suf_len = 0;
					int uri_len = 0;

					ws_sock_del();

					uri_len = get_uri(data, len, &uri, &suf, &suf_len, &get_root );

					hdr = suf_to_hdr(suf, suf_len, &hdr_sz);
					
					//DBG("client->send_timeout %d\n", client->send_timeout );
					if( do_websock(&uri, uri_len, hdr, hdr_sz, data, len, NULL /*char *out*/, suf ) > 0){
						//nb_free();
					}else{
						nb_free();
						
						if(suf != NULL && ( (!strcmp(suf, ".lua")) || (!strcmp(suf, ".cgi")) ) ){
							DBG("lua cgi = %s\n", uri);
							do_lua(&uri, uri_len, hdr, hdr_sz, /*data, len,*/ NULL /*char *out*/, L, &get_root );
						}else
						if( do_file(&uri, uri_len, hdr, hdr_sz, /*data, len,*/ NULL/*char *out*/ ) > 0){
						}else
						if( do_something(&uri, uri_len, hdr, hdr_sz, NULL/*&buf_len*/) > 0){
						}else 
						{
							//nb_free();
							//ws_sock_del();
							do_404(&uri, uri_len, hdr, hdr_sz, /*data, len,*/ NULL/*out*/);
						}
					}
					if(uri != NULL){
						free(uri);
						uri = NULL;
						uri_len = 0;
					}
					if(suf != NULL){
						free(suf);
						suf = NULL;
					}
					//if(get_root != NULL){
						get_list_free(&get_root);
					//}
					
                }
            }
			nb_free();
            //netbuf_delete(nb);
        }
		nc_free(&client, "Closing connection (client)\n");
		//usleep(50);
		ws_task(L);
		//usleep(10);

		if(is_httpd_run == 2)
			is_httpd_run = 3;
    }
	
	nb_free();
	nc_free(&client, "Closing connection (client) on http exit\n");
	ws_sock_del();
	//if(get_root != NULL){
		get_list_free(&get_root);
	//}

	nc_free(&nc, "Closing connection (main nc)\n");
	
	if(is_httpd_run == 3)
		is_httpd_run = 2;

//}while( is_httpd_run == 4);

	DBG("httpd_task 2 Free mem: %d\n",xPortGetFreeHeapSize());
	DBG("exit httpd_task: \nws_client=%x, ws_uri=%x, \nnc=%x, client=%x\nnb=%x, get_root=%x\n",
		ws_client, ws_uri, nc, client, nb, get_root
	);
	return 0;
}

int httpd_task_loop_run(lua_State* L){
	is_httpd_run = 1;
	int res;

	do{
		if(is_httpd_run == 4) {
			is_httpd_run = 1;
			sleep(2);
		}
		
		res = httpd_task(L);
	}while( is_httpd_run == 4 );
	
	return res;
}



int (*cb_httpd)(lua_State *L) = NULL;


int httpd_task_cb_run(lua_State* L){
	is_httpd_run = 2;

	cb_httpd = httpd_task;
	return 0;
}


void sdk_sta_status_set(int);

int httpd_task_stop(lua_State* L){
	sdk_sta_status_set(0);
	
	is_httpd_run = 0;

	cb_httpd = NULL;

//	client->send_timeout=1;
//	client->recv_timeout=1;

//	nc->send_timeout=1;
//	nc->recv_timeout=1;
	
	return 0;
}
#endif


/*
int httpd_start(lua_State* L) {
//	static TaskHandle_t xHandle = NULL;
//	if(xHandle != NULL) 
//		vTaskDelete( xHandle );
	lua_settop(L,0);
	lua_pushcfunction(L, &httpd_task);
	int r = new_thread(L, 1);
	DBG("httpd started status=%d\n", r);
//	xTaskCreate(&httpd_task, "httpd", 312, NULL, 2, &xHandle);

	return 0;
}
*/

static int httpd_recv_timeout(lua_State* L) {
	int timeout = recv_timeout;
	if(lua_gettop(L) > 0){
		recv_timeout = luaL_optinteger(L, 1, DEF_RECV_TIMEOUT);
		if(recv_timeout < 0) recv_timeout = DEF_RECV_TIMEOUT;
	}
	lua_pushinteger(L, timeout);
	return 1;
}

static int httpd_send_timeout(lua_State* L) {
	int timeout = send_timeout;
	if(lua_gettop(L) > 0){
		send_timeout = luaL_optinteger(L, 1, DEF_SEND_TIMEOUT);
		if(send_timeout < 0) send_timeout = DEF_SEND_TIMEOUT;
	}
	lua_pushinteger(L, timeout);
	return 1;
}



#include "modules.h"


const LUA_REG_TYPE httpd_map[] = {
//		{ LSTRKEY( "httpd" ),		LFUNCVAL( net_httpd_start ) },
//		{ LSTRKEY( "httpd" ),		LFUNCVAL( httpd_start ) },
		{ LSTRKEY( "loop" ),		LFUNCVAL( httpd_task_loop_run ) },
		{ LSTRKEY( "add_to_callbacks" ),	LFUNCVAL( httpd_task_cb_run ) },
		
		{ LSTRKEY( "stop" ),			LFUNCVAL( httpd_task_stop ) },
		
		{ LSTRKEY( "recv_timeout" ),	LFUNCVAL( httpd_recv_timeout ) },
		{ LSTRKEY( "send_timeout" ),	LFUNCVAL( httpd_send_timeout ) },
		
		{ LNILKEY, LNILVAL }
};

int luaopen_httpd( lua_State *L ) {
	cb_httpd = NULL;
	
	return 0;
}


MODULE_REGISTER_MAPPED(HTTPD, httpd, httpd_map, luaopen_httpd);


