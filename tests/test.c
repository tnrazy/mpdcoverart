/*
 *     Filename:  test.c
 *         Mail:  tn.razy@gmail.com
 *
 *  Description:  
 *
 */

#include "../src/ui/ui.h"
#include "msg.h"
#include "list.h"
#include "http.h"
#include "coverfetch.h"
#include "setting.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

struct pos
{
    int x;
    int y;
};
void pos_free(void *ptr)
{
    printf("free pos\n");
    if(ptr == NULL)
        return;
    free(ptr);
}

int main(int argc, const char *argv[])
{
    goto http_test;
    struct pos p1 = {1, 2};
    struct pos p2 = {2, 3};
    struct pos p3 = {99, 100};

    Dlist *list = Dlist_new();
    Dlist_entity *e;
    value_free vf = pos_free;

    e = Dlist_entity_new(&p1, sizeof(struct pos), vf);
    list = Dlist_add(list, "1", e);

    e = Dlist_entity_new(&p2, sizeof(struct pos), vf);
    list = Dlist_add(list, "2", e);

    e = Dlist_entity_new(&p3, sizeof(struct pos), vf);
    list = Dlist_add(list, "3", e);

    list = Dlist_del(list, "3");
    printf("size: %d\n", Dlist_get_size(list));

    list = Dlist_insert(list, "6", NULL, NULL);
    Dlist *res = Dlist_find(list, "6");
    struct pos *pp1 = res == NULL ? NULL : (res->entity ? res->entity->data : NULL);
    if(pp1)
    {
        printf("key: %s\n", res->key);
        /*printf("pp1->x = %d\n", pp1->x);*/
        /*printf("pp1->y = %d\n", pp1->y);*/
    }
    
    _DEBUG("Size: %d", Dlist_get_size(list));
    Dlist_free(list);

    return 0;





    //struct http_req *req;
    //struct http_res *resp;

http_test:

    /*req = http_compile("http://www.douban.com/search", HTTP_MTD_POST, http_req_hdr_post, HTTP_CXT_X_FORM, "?search_text=Lenka");*/
    /*req = http_compile("http://img1.douban.com/lpic/s3259484.jpg", HTTP_MTD_GET, http_req_hdr_get, 0, 0);*/
    /*req = http_compile("http://img1.douban.com/lpic/s4411596.jpg", HTTP_MTD_GET, http_req_hdr_get, 0, 0);*/
    //req = http_compile("http://www.douban.com/search?search_text=Lenka+Two", HTTP_MTD_GET, http_req_hdr_get, 0, 0);
    /*req = http_compile("http://img1.douban.com/lpic/ssss3259484.jpg", HTTP_MTD_HEAD, http_req_hdr_hdr, 0, 0);*/
    //req = http_compile("http://www.twitter.com", HTTP_MTD_HEAD, http_req_hdr_hdr, 0, 0);
    
    //int connfd = http_connect(req->addr, 10);
    //resp = http_exec(connfd, req, 3);

    //http_fetch(connfd, resp, "test");

    /*int fd = open("spic.jpg", O_CREAT | O_WRONLY);*/

    /*if(fd == -1)*/
    /*{*/
        /*printf("error\n");*/
        /*exit(1);*/
    /*}*/

    /*int num = write(fd, resp->data, resp->length);*/

    //printf("length: %ld\n", resp->length);

    //http_closeconn(connfd);
    //http_req_free(req);
    //http_res_free(resp);

    printf("name: %s\n", cfg_get_skinname());
    printf("X: %d\n", cfg_get_pos_x());
    printf("Y: %d\n", cfg_get_pos_y());
    printf("lock: %d\n", cfg_get_pos_lock());
    ui_get_current_skin();
    /*doubancover("Lady gaga", "/home/tnrazy/");*/

    ui_load();
    //http_getfile("img1.douban.comlspic/s4642994.jpg", "test");

    return 0;
}
