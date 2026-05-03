#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- STRUCTURES ---------- */
struct Victim {
    char id[10];
    char name[50];
    int age;
    int injury;
    char location[50];
    char shelterId[10];
};
struct Shelter {
    char id[10];
    char name[50];
    char location[50];
    int capacity;
    int current;
};
struct Resource {
    char id[10];
    char name[50];
    int quantity;
    char type[30];
};
struct Volunteer {
    char id[10];
    char name[50];
    char role[30];
};
struct Distribution {
    char victimId[10];
    char resourceId[10];
    int quantity;
};

/* ---------- GLOBALS ---------- */
GtkWidget *main_window, *stack, *status_bar;
GtkWidget *v_id, *v_name, *v_age, *v_injury, *v_location, *v_list;
GtkWidget *s_id, *s_name, *s_location, *s_capacity, *s_list;
GtkWidget *r_id, *r_name, *r_type, *r_quantity, *r_list;
GtkWidget *vol_id, *vol_name, *vol_role, *vol_list;
GtkWidget *d_victimid, *d_resourceid, *d_quantity, *d_list;
GtkWidget *summary_view;

/* ---------- HELPERS ---------- */
void set_status(const char *msg) {
    gtk_statusbar_pop(GTK_STATUSBAR(status_bar), 1);
    gtk_statusbar_push(GTK_STATUSBAR(status_bar), 1, msg);
}
void normalizeID(char id[]) {
    while(id[0]==' ') memmove(id,id+1,strlen(id));
    int len=strlen(id);
    while(len>0&&id[len-1]==' '){id[len-1]='\0';len--;}
    for(int i=0;id[i];i++) if(id[i]>='a'&&id[i]<='z') id[i]-=32;
}
int isValidRole(char role[]) {
    return (strcmp(role,"Medical")==0||strcmp(role,"Food")==0||strcmp(role,"Transport")==0);
}
void clear_list(GtkWidget *tv) {
    gtk_list_store_clear(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(tv))));
}
GtkWidget* make_entry(const char *ph) {
    GtkWidget *e=gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(e),ph);
    return e;
}
GtkWidget* make_button(const char *label, GCallback cb, gpointer data) {
    GtkWidget *b=gtk_button_new_with_label(label);
    if(cb) g_signal_connect(b,"clicked",cb,data);
    return b;
}
GtkWidget* make_treeview(GtkListStore **sout, int ncols, const char **cols) {
    GType types[10];
    for(int i=0;i<ncols;i++) types[i]=G_TYPE_STRING;
    GtkListStore *store=gtk_list_store_newv(ncols,types);
    *sout=store;
    GtkWidget *tv=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    for(int i=0;i<ncols;i++){
        GtkCellRenderer *r=gtk_cell_renderer_text_new();
        GtkTreeViewColumn *c=gtk_tree_view_column_new_with_attributes(cols[i],r,"text",i,NULL);
        gtk_tree_view_column_set_resizable(c,TRUE);
        gtk_tree_view_column_set_min_width(c,80);
        gtk_tree_view_append_column(GTK_TREE_VIEW(tv),c);
    }
    return tv;
}

/* ============================
   VICTIM
   ============================ */
void refresh_victim_list() {
    clear_list(v_list);
    FILE *fp=fopen("victim.dat","rb"); if(!fp) return;
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(v_list)));
    GtkTreeIter iter; struct Victim v;
    while(fread(&v,sizeof(v),1,fp)){
        char age[10],inj[10]; sprintf(age,"%d",v.age); sprintf(inj,"%d",v.injury);
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store,&iter,0,v.id,1,v.name,2,age,3,inj,4,v.location,5,v.shelterId,-1);
    }
    fclose(fp);
}

void on_add_victim(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(v_id));
    const char *name=gtk_entry_get_text(GTK_ENTRY(v_name));
    const char *age=gtk_entry_get_text(GTK_ENTRY(v_age));
    const char *inj=gtk_entry_get_text(GTK_ENTRY(v_injury));
    const char *loc=gtk_entry_get_text(GTK_ENTRY(v_location));
    if(!strlen(id)||!strlen(name)||!strlen(age)||!strlen(inj)||!strlen(loc)){set_status("Error: Fill in all fields.");return;}

    struct Victim v;
    strncpy(v.id,id,9);v.id[9]=0;normalizeID(v.id);
    strncpy(v.name,name,49);v.name[49]=0;
    v.age=atoi(age);v.injury=atoi(inj);
    strncpy(v.location,loc,49);v.location[49]=0;normalizeID(v.location);
    if(v.age<=0||v.injury<=0){set_status("Error: Age and injury must be positive.");return;}

    FILE *ck=fopen("victim.dat","rb");
    if(ck){struct Victim t;while(fread(&t,sizeof(t),1,ck))if(strcmp(t.id,v.id)==0){set_status("Error: ID already exists.");fclose(ck);return;}fclose(ck);}

    FILE *sf=fopen("shelter.dat","rb+"); struct Shelter s; int assigned=0; long pos; strcpy(v.shelterId,"NONE");
    if(sf){
        while(fread(&s,sizeof(s),1,sf)) if(strcmp(s.location,v.location)==0&&s.current<s.capacity){strcpy(v.shelterId,s.id);pos=ftell(sf)-sizeof(s);assigned=1;break;}
        if(!assigned){rewind(sf);while(fread(&s,sizeof(s),1,sf))if(s.current<s.capacity){strcpy(v.shelterId,s.id);pos=ftell(sf)-sizeof(s);assigned=1;break;}}
        if(assigned){s.current++;fseek(sf,pos,SEEK_SET);fwrite(&s,sizeof(s),1,sf);}
        fclose(sf);
    }
    FILE *fp=fopen("victim.dat","ab");
    if(fp){fwrite(&v,sizeof(v),1,fp);fclose(fp);}
    char msg[100]; sprintf(msg,"Victim %s added. Shelter: %s",v.id,v.shelterId); set_status(msg);
    gtk_entry_set_text(GTK_ENTRY(v_id),"");gtk_entry_set_text(GTK_ENTRY(v_name),"");
    gtk_entry_set_text(GTK_ENTRY(v_age),"");gtk_entry_set_text(GTK_ENTRY(v_injury),"");
    gtk_entry_set_text(GTK_ENTRY(v_location),"");
    refresh_victim_list();
}

void on_update_victim(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(v_id));
    if(!strlen(id)){set_status("Error: Enter Victim ID to update.");return;}
    const char *name=gtk_entry_get_text(GTK_ENTRY(v_name));
    const char *age=gtk_entry_get_text(GTK_ENTRY(v_age));
    const char *inj=gtk_entry_get_text(GTK_ENTRY(v_injury));
    const char *loc=gtk_entry_get_text(GTK_ENTRY(v_location));
    if(!strlen(name)||!strlen(age)||!strlen(inj)||!strlen(loc)){set_status("Error: Fill in all fields for update.");return;}

    char vid[10]; strncpy(vid,id,9);vid[9]=0;normalizeID(vid);
    FILE *fp=fopen("victim.dat","rb+");
    if(!fp){set_status("Error: No victim data found.");return;}

    struct Victim v; int found=0;
    while(fread(&v,sizeof(v),1,fp)){
        if(strcmp(v.id,vid)==0){
            char oldShelter[10]; strcpy(oldShelter,v.shelterId);
            strncpy(v.name,name,49);v.name[49]=0;
            v.age=atoi(age); v.injury=atoi(inj);
            strncpy(v.location,loc,49);v.location[49]=0;normalizeID(v.location);

            /* Shelter reassign */
            FILE *sf=fopen("shelter.dat","rb+");
            if(sf){
                struct Shelter s; int assigned=0; long pos; char newShelter[10]; strcpy(newShelter,oldShelter);
                while(fread(&s,sizeof(s),1,sf)) if(strcmp(s.location,v.location)==0&&s.current<s.capacity){strcpy(newShelter,s.id);assigned=1;break;}
                if(!assigned){rewind(sf);while(fread(&s,sizeof(s),1,sf))if(s.current<s.capacity){strcpy(newShelter,s.id);assigned=1;break;}}
                if(assigned&&strcmp(oldShelter,newShelter)!=0){
                    rewind(sf);
                    while(fread(&s,sizeof(s),1,sf)) if(strcmp(s.id,newShelter)==0){s.current++;pos=ftell(sf)-sizeof(s);fseek(sf,pos,SEEK_SET);fwrite(&s,sizeof(s),1,sf);break;}
                    rewind(sf);
                    while(fread(&s,sizeof(s),1,sf)) if(strcmp(s.id,oldShelter)==0){if(s.current>0)s.current--;pos=ftell(sf)-sizeof(s);fseek(sf,pos,SEEK_SET);fwrite(&s,sizeof(s),1,sf);break;}
                    strcpy(v.shelterId,newShelter);
                }
                fclose(sf);
            }
            fseek(fp,-(long)sizeof(v),SEEK_CUR);
            fwrite(&v,sizeof(v),1,fp);
            found=1; break;
        }
    }
    fclose(fp);
    set_status(found?"Victim updated successfully.":"Error: Victim not found.");
    refresh_victim_list();
}

void on_delete_victim(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(v_id));
    if(!strlen(id)){set_status("Error: Enter Victim ID.");return;}
    char vid[10];strncpy(vid,id,9);vid[9]=0;normalizeID(vid);
    FILE *fp=fopen("victim.dat","rb"),*tmp=fopen("victim_tmp.dat","wb");
    if(!fp||!tmp){set_status("Error: File operation failed.");if(fp)fclose(fp);if(tmp)fclose(tmp);return;}
    struct Victim v;int found=0;
    while(fread(&v,sizeof(v),1,fp)){
        if(strcmp(v.id,vid)!=0)fwrite(&v,sizeof(v),1,tmp);
        else{found=1;FILE *sf=fopen("shelter.dat","rb+");if(sf){struct Shelter s;while(fread(&s,sizeof(s),1,sf))if(strcmp(s.id,v.shelterId)==0){if(s.current>0)s.current--;fseek(sf,-(long)sizeof(s),SEEK_CUR);fwrite(&s,sizeof(s),1,sf);break;}fclose(sf);}}
    }
    fclose(fp);fclose(tmp);remove("victim.dat");rename("victim_tmp.dat","victim.dat");
    set_status(found?"Victim deleted.":"Error: Victim not found.");
    refresh_victim_list();
}

void on_search_victim(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(v_id));
    if(!strlen(id)){set_status("Error: Enter Victim ID.");return;}
    char vid[10];strncpy(vid,id,9);vid[9]=0;normalizeID(vid);
    FILE *fp=fopen("victim.dat","rb");if(!fp){set_status("Error: No data.");return;}
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(v_list)));
    gtk_list_store_clear(store);GtkTreeIter iter;struct Victim v;int found=0;
    while(fread(&v,sizeof(v),1,fp)){
        if(strcmp(v.id,vid)==0){
            char age[10],inj[10];sprintf(age,"%d",v.age);sprintf(inj,"%d",v.injury);
            gtk_list_store_append(store,&iter);
            gtk_list_store_set(store,&iter,0,v.id,1,v.name,2,age,3,inj,4,v.location,5,v.shelterId,-1);
            found=1;break;
        }
    }
    fclose(fp);set_status(found?"Victim found.":"Error: Not found.");
}

void on_sort_victims(GtkWidget *btn, gpointer data) {
    FILE *fp=fopen("victim.dat","rb");if(!fp){set_status("Error: No data.");return;}
    struct Victim *arr=NULL;int n=0;struct Victim tmp;
    while(fread(&tmp,sizeof(tmp),1,fp)){arr=realloc(arr,(n+1)*sizeof(struct Victim));arr[n++]=tmp;}
    fclose(fp);
    for(int i=0;i<n-1;i++) for(int j=0;j<n-i-1;j++) if(arr[j].injury<arr[j+1].injury){struct Victim t=arr[j];arr[j]=arr[j+1];arr[j+1]=t;}
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(v_list)));
    gtk_list_store_clear(store);GtkTreeIter iter;
    for(int i=0;i<n;i++){
        char age[10],inj[10];sprintf(age,"%d",arr[i].age);sprintf(inj,"%d",arr[i].injury);
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store,&iter,0,arr[i].id,1,arr[i].name,2,age,3,inj,4,arr[i].location,5,arr[i].shelterId,-1);
    }
    free(arr);set_status("Sorted by injury level (highest first).");
}

/* ============================
   SHELTER
   ============================ */
void refresh_shelter_list() {
    clear_list(s_list);
    FILE *fp=fopen("shelter.dat","rb");if(!fp)return;
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(s_list)));
    GtkTreeIter iter;struct Shelter s;
    while(fread(&s,sizeof(s),1,fp)){
        char cap[10],cur[10],avail[10];
        sprintf(cap,"%d",s.capacity);sprintf(cur,"%d",s.current);sprintf(avail,"%d",s.capacity-s.current);
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store,&iter,0,s.id,1,s.name,2,s.location,3,cap,4,cur,5,avail,6,(s.current<s.capacity)?"Available":"Full",-1);
    }
    fclose(fp);
}

void on_add_shelter(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(s_id));
    const char *nm=gtk_entry_get_text(GTK_ENTRY(s_name));
    const char *loc=gtk_entry_get_text(GTK_ENTRY(s_location));
    const char *cap=gtk_entry_get_text(GTK_ENTRY(s_capacity));
    if(!strlen(id)||!strlen(nm)||!strlen(loc)||!strlen(cap)){set_status("Error: Fill in all fields.");return;}
    struct Shelter s;
    strncpy(s.id,id,9);s.id[9]=0;normalizeID(s.id);
    strncpy(s.name,nm,49);s.name[49]=0;
    strncpy(s.location,loc,49);s.location[49]=0;normalizeID(s.location);
    s.capacity=atoi(cap);s.current=0;
    if(s.capacity<=0){set_status("Error: Capacity must be positive.");return;}
    FILE *ck=fopen("shelter.dat","rb");
    if(ck){struct Shelter t;while(fread(&t,sizeof(t),1,ck))if(strcmp(t.id,s.id)==0){set_status("Error: ID already exists.");fclose(ck);return;}fclose(ck);}
    FILE *fp=fopen("shelter.dat","ab");if(fp){fwrite(&s,sizeof(s),1,fp);fclose(fp);}
    set_status("Shelter added successfully.");
    gtk_entry_set_text(GTK_ENTRY(s_id),"");gtk_entry_set_text(GTK_ENTRY(s_name),"");
    gtk_entry_set_text(GTK_ENTRY(s_location),"");gtk_entry_set_text(GTK_ENTRY(s_capacity),"");
    refresh_shelter_list();
}

void on_update_shelter(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(s_id));
    const char *cap=gtk_entry_get_text(GTK_ENTRY(s_capacity));
    if(!strlen(id)||!strlen(cap)){set_status("Error: Enter Shelter ID and new Capacity.");return;}
    char sid[10];strncpy(sid,id,9);sid[9]=0;normalizeID(sid);
    FILE *fp=fopen("shelter.dat","rb+");if(!fp){set_status("Error: No shelter data.");return;}
    struct Shelter s;int found=0;
    while(fread(&s,sizeof(s),1,fp)){
        if(strcmp(s.id,sid)==0){
            int newcap=atoi(cap);
            if(newcap<=0){set_status("Error: Capacity must be positive.");fclose(fp);return;}
            if(newcap<s.current){set_status("Error: Cannot reduce below current occupants.");fclose(fp);return;}
            s.capacity=newcap;
            fseek(fp,-(long)sizeof(s),SEEK_CUR);fwrite(&s,sizeof(s),1,fp);
            found=1;break;
        }
    }
    fclose(fp);
    set_status(found?"Shelter capacity updated.":"Error: Shelter not found.");
    refresh_shelter_list();
}

void on_delete_shelter(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(s_id));
    if(!strlen(id)){set_status("Error: Enter Shelter ID.");return;}
    char sid[10];strncpy(sid,id,9);sid[9]=0;normalizeID(sid);
    FILE *fp=fopen("shelter.dat","rb"),*tmp=fopen("shelter_tmp.dat","wb");
    if(!fp||!tmp){set_status("Error: File operation failed.");if(fp)fclose(fp);if(tmp)fclose(tmp);return;}
    struct Shelter s;int found=0;
    while(fread(&s,sizeof(s),1,fp)){
        if(strcmp(s.id,sid)!=0)fwrite(&s,sizeof(s),1,tmp);
        else{if(s.current>0){set_status("Error: Cannot delete occupied shelter.");fclose(fp);fclose(tmp);remove("shelter_tmp.dat");return;}found=1;}
    }
    fclose(fp);fclose(tmp);remove("shelter.dat");rename("shelter_tmp.dat","shelter.dat");
    set_status(found?"Shelter deleted.":"Error: Not found.");
    refresh_shelter_list();
}

void on_search_shelter(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(s_id));
    if(!strlen(id)){set_status("Error: Enter Shelter ID.");return;}
    char sid[10];strncpy(sid,id,9);sid[9]=0;normalizeID(sid);
    FILE *fp=fopen("shelter.dat","rb");if(!fp){set_status("Error: No data.");return;}
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(s_list)));
    gtk_list_store_clear(store);GtkTreeIter iter;struct Shelter s;int found=0;
    while(fread(&s,sizeof(s),1,fp)){
        if(strcmp(s.id,sid)==0){
            char cap[10],cur[10],avail[10];
            sprintf(cap,"%d",s.capacity);sprintf(cur,"%d",s.current);sprintf(avail,"%d",s.capacity-s.current);
            gtk_list_store_append(store,&iter);
            gtk_list_store_set(store,&iter,0,s.id,1,s.name,2,s.location,3,cap,4,cur,5,avail,6,(s.current<s.capacity)?"Available":"Full",-1);
            found=1;break;
        }
    }
    fclose(fp);set_status(found?"Shelter found.":"Error: Not found.");
}

/* ============================
   RESOURCE
   ============================ */
void refresh_resource_list() {
    clear_list(r_list);
    FILE *fp=fopen("resources.dat","rb");if(!fp)return;
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(r_list)));
    GtkTreeIter iter;struct Resource r;
    while(fread(&r,sizeof(r),1,fp)){
        char qty[10];sprintf(qty,"%d",r.quantity);
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store,&iter,0,r.id,1,r.name,2,r.type,3,qty,-1);
    }
    fclose(fp);
}

void on_add_resource(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(r_id));
    const char *nm=gtk_entry_get_text(GTK_ENTRY(r_name));
    const char *tp=gtk_entry_get_text(GTK_ENTRY(r_type));
    const char *qty=gtk_entry_get_text(GTK_ENTRY(r_quantity));
    if(!strlen(id)||!strlen(nm)||!strlen(tp)||!strlen(qty)){set_status("Error: Fill in all fields.");return;}
    struct Resource r;
    strncpy(r.id,id,9);r.id[9]=0;normalizeID(r.id);
    strncpy(r.name,nm,49);r.name[49]=0;
    strncpy(r.type,tp,29);r.type[29]=0;
    r.quantity=atoi(qty);
    if(r.quantity<=0){set_status("Error: Quantity must be positive.");return;}
    FILE *ck=fopen("resources.dat","rb");
    if(ck){struct Resource t;while(fread(&t,sizeof(t),1,ck))if(strcmp(t.id,r.id)==0){set_status("Error: ID already exists.");fclose(ck);return;}fclose(ck);}
    FILE *fp=fopen("resources.dat","ab");if(fp){fwrite(&r,sizeof(r),1,fp);fclose(fp);}
    set_status("Resource added successfully.");
    gtk_entry_set_text(GTK_ENTRY(r_id),"");gtk_entry_set_text(GTK_ENTRY(r_name),"");
    gtk_entry_set_text(GTK_ENTRY(r_type),"");gtk_entry_set_text(GTK_ENTRY(r_quantity),"");
    refresh_resource_list();
}

void on_update_resource(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(r_id));
    const char *qty=gtk_entry_get_text(GTK_ENTRY(r_quantity));
    if(!strlen(id)||!strlen(qty)){set_status("Error: Enter Resource ID and new Quantity.");return;}
    char rid[10];strncpy(rid,id,9);rid[9]=0;normalizeID(rid);
    FILE *fp=fopen("resources.dat","rb+");if(!fp){set_status("Error: No resource data.");return;}
    struct Resource r;int found=0;
    while(fread(&r,sizeof(r),1,fp)){
        if(strcmp(r.id,rid)==0){
            int newqty=atoi(qty);
            if(newqty<0){set_status("Error: Quantity cannot be negative.");fclose(fp);return;}
            r.quantity=newqty;
            fseek(fp,-(long)sizeof(r),SEEK_CUR);fwrite(&r,sizeof(r),1,fp);
            found=1;break;
        }
    }
    fclose(fp);
    set_status(found?"Resource quantity updated.":"Error: Resource not found.");
    refresh_resource_list();
}

void on_delete_resource(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(r_id));
    if(!strlen(id)){set_status("Error: Enter Resource ID.");return;}
    char rid[10];strncpy(rid,id,9);rid[9]=0;normalizeID(rid);
    FILE *fp=fopen("resources.dat","rb"),*tmp=fopen("res_tmp.dat","wb");
    if(!fp||!tmp){set_status("Error: File operation failed.");if(fp)fclose(fp);if(tmp)fclose(tmp);return;}
    struct Resource r;int found=0;
    while(fread(&r,sizeof(r),1,fp)){if(strcmp(r.id,rid)!=0)fwrite(&r,sizeof(r),1,tmp);else found=1;}
    fclose(fp);fclose(tmp);remove("resources.dat");rename("res_tmp.dat","resources.dat");
    set_status(found?"Resource deleted.":"Error: Not found.");
    refresh_resource_list();
}

void on_search_resource(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(r_id));
    if(!strlen(id)){set_status("Error: Enter Resource ID.");return;}
    char rid[10];strncpy(rid,id,9);rid[9]=0;normalizeID(rid);
    FILE *fp=fopen("resources.dat","rb");if(!fp){set_status("Error: No data.");return;}
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(r_list)));
    gtk_list_store_clear(store);GtkTreeIter iter;struct Resource r;int found=0;
    while(fread(&r,sizeof(r),1,fp)){
        if(strcmp(r.id,rid)==0){
            char qty[10];sprintf(qty,"%d",r.quantity);
            gtk_list_store_append(store,&iter);
            gtk_list_store_set(store,&iter,0,r.id,1,r.name,2,r.type,3,qty,-1);
            found=1;break;
        }
    }
    fclose(fp);set_status(found?"Resource found.":"Error: Not found.");
}

/* ============================
   VOLUNTEER
   ============================ */
void refresh_volunteer_list() {
    clear_list(vol_list);
    FILE *fp=fopen("volunteers.dat","rb");if(!fp)return;
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(vol_list)));
    GtkTreeIter iter;struct Volunteer v;
    while(fread(&v,sizeof(v),1,fp)){
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store,&iter,0,v.id,1,v.name,2,v.role,-1);
    }
    fclose(fp);
}

void on_add_volunteer(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(vol_id));
    const char *nm=gtk_entry_get_text(GTK_ENTRY(vol_name));
    const char *role=gtk_entry_get_text(GTK_ENTRY(vol_role));
    if(!strlen(id)||!strlen(nm)||!strlen(role)){set_status("Error: Fill in all fields.");return;}
    struct Volunteer v;
    strncpy(v.id,id,9);v.id[9]=0;normalizeID(v.id);
    strncpy(v.name,nm,49);v.name[49]=0;
    strncpy(v.role,role,29);v.role[29]=0;
    if(!isValidRole(v.role)){set_status("Error: Role must be Medical, Food, or Transport.");return;}
    FILE *ck=fopen("volunteers.dat","rb");
    if(ck){struct Volunteer t;while(fread(&t,sizeof(t),1,ck))if(strcmp(t.id,v.id)==0){set_status("Error: ID already exists.");fclose(ck);return;}fclose(ck);}
    FILE *fp=fopen("volunteers.dat","ab");if(fp){fwrite(&v,sizeof(v),1,fp);fclose(fp);}
    set_status("Volunteer added successfully.");
    gtk_entry_set_text(GTK_ENTRY(vol_id),"");gtk_entry_set_text(GTK_ENTRY(vol_name),"");gtk_entry_set_text(GTK_ENTRY(vol_role),"");
    refresh_volunteer_list();
}

void on_update_volunteer(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(vol_id));
    const char *role=gtk_entry_get_text(GTK_ENTRY(vol_role));
    if(!strlen(id)||!strlen(role)){set_status("Error: Enter Volunteer ID and new Role.");return;}
    char vid[10];strncpy(vid,id,9);vid[9]=0;normalizeID(vid);
    char newrole[30];strncpy(newrole,role,29);newrole[29]=0;
    if(!isValidRole(newrole)){set_status("Error: Role must be Medical, Food, or Transport.");return;}
    FILE *fp=fopen("volunteers.dat","rb+");if(!fp){set_status("Error: No volunteer data.");return;}
    struct Volunteer v;int found=0;
    while(fread(&v,sizeof(v),1,fp)){
        if(strcmp(v.id,vid)==0){
            strcpy(v.role,newrole);
            fseek(fp,-(long)sizeof(v),SEEK_CUR);fwrite(&v,sizeof(v),1,fp);
            found=1;break;
        }
    }
    fclose(fp);
    set_status(found?"Volunteer role updated.":"Error: Volunteer not found.");
    refresh_volunteer_list();
}

void on_delete_volunteer(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(vol_id));
    if(!strlen(id)){set_status("Error: Enter Volunteer ID.");return;}
    char vid[10];strncpy(vid,id,9);vid[9]=0;normalizeID(vid);
    FILE *fp=fopen("volunteers.dat","rb"),*tmp=fopen("vol_tmp.dat","wb");
    if(!fp||!tmp){set_status("Error: File operation failed.");if(fp)fclose(fp);if(tmp)fclose(tmp);return;}
    struct Volunteer v;int found=0;
    while(fread(&v,sizeof(v),1,fp)){if(strcmp(v.id,vid)!=0)fwrite(&v,sizeof(v),1,tmp);else found=1;}
    fclose(fp);fclose(tmp);remove("volunteers.dat");rename("vol_tmp.dat","volunteers.dat");
    set_status(found?"Volunteer deleted.":"Error: Not found.");
    refresh_volunteer_list();
}

void on_search_volunteer(GtkWidget *btn, gpointer data) {
    const char *id=gtk_entry_get_text(GTK_ENTRY(vol_id));
    if(!strlen(id)){set_status("Error: Enter Volunteer ID.");return;}
    char vid[10];strncpy(vid,id,9);vid[9]=0;normalizeID(vid);
    FILE *fp=fopen("volunteers.dat","rb");if(!fp){set_status("Error: No data.");return;}
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(vol_list)));
    gtk_list_store_clear(store);GtkTreeIter iter;struct Volunteer v;int found=0;
    while(fread(&v,sizeof(v),1,fp)){
        if(strcmp(v.id,vid)==0){
            gtk_list_store_append(store,&iter);
            gtk_list_store_set(store,&iter,0,v.id,1,v.name,2,v.role,-1);
            found=1;break;
        }
    }
    fclose(fp);set_status(found?"Volunteer found.":"Error: Not found.");
}

/* ============================
   DISTRIBUTION
   ============================ */
void refresh_distribution_list() {
    clear_list(d_list);
    FILE *fp=fopen("distribution.dat","rb");if(!fp)return;
    GtkListStore *store=GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(d_list)));
    GtkTreeIter iter;struct Distribution d;
    while(fread(&d,sizeof(d),1,fp)){
        char qty[10];sprintf(qty,"%d",d.quantity);
        gtk_list_store_append(store,&iter);
        gtk_list_store_set(store,&iter,0,d.victimId,1,d.resourceId,2,qty,-1);
    }
    fclose(fp);
}

void on_assign_resource(GtkWidget *btn, gpointer data) {
    const char *vid=gtk_entry_get_text(GTK_ENTRY(d_victimid));
    const char *rid=gtk_entry_get_text(GTK_ENTRY(d_resourceid));
    const char *qty=gtk_entry_get_text(GTK_ENTRY(d_quantity));
    if(!strlen(vid)||!strlen(rid)||!strlen(qty)){set_status("Error: Fill in all fields.");return;}
    struct Distribution d;
    strncpy(d.victimId,vid,9);d.victimId[9]=0;normalizeID(d.victimId);
    strncpy(d.resourceId,rid,9);d.resourceId[9]=0;normalizeID(d.resourceId);
    d.quantity=atoi(qty);
    if(d.quantity<=0){set_status("Error: Quantity must be positive.");return;}
    FILE *vf=fopen("victim.dat","rb");int vfound=0;
    if(vf){struct Victim v;while(fread(&v,sizeof(v),1,vf))if(strcmp(v.id,d.victimId)==0){vfound=1;break;}fclose(vf);}
    if(!vfound){set_status("Error: Victim ID not found.");return;}
    FILE *rf=fopen("resources.dat","rb+");if(!rf){set_status("Error: Resource file error.");return;}
    struct Resource r;int rfound=0;
    while(fread(&r,sizeof(r),1,rf)){
        if(strcmp(r.id,d.resourceId)==0){
            if(r.quantity<d.quantity){set_status("Error: Not enough resources available.");fclose(rf);return;}
            r.quantity-=d.quantity;fseek(rf,-(long)sizeof(r),SEEK_CUR);fwrite(&r,sizeof(r),1,rf);rfound=1;break;
        }
    }
    fclose(rf);
    if(!rfound){set_status("Error: Resource ID not found.");return;}
    FILE *df=fopen("distribution.dat","ab");if(df){fwrite(&d,sizeof(d),1,df);fclose(df);}
    set_status("Resource distributed successfully.");
    gtk_entry_set_text(GTK_ENTRY(d_victimid),"");gtk_entry_set_text(GTK_ENTRY(d_resourceid),"");gtk_entry_set_text(GTK_ENTRY(d_quantity),"");
    refresh_distribution_list();refresh_resource_list();
}

/* ============================
   SUMMARY
   ============================ */
void on_refresh_summary(GtkWidget *btn, gpointer data) {
    int victims=0,shelters=0,resources=0,volunteers=0,totalCap=0,occupied=0,fullShelters=0,maxInj=-1;
    struct Victim v;struct Shelter s;struct Resource r;struct Volunteer vol;FILE *fp;
    fp=fopen("victim.dat","rb");if(fp){while(fread(&v,sizeof(v),1,fp)){victims++;if(v.injury>maxInj)maxInj=v.injury;}fclose(fp);}
    fp=fopen("shelter.dat","rb");if(fp){while(fread(&s,sizeof(s),1,fp)){shelters++;totalCap+=s.capacity;occupied+=s.current;if(s.current==s.capacity)fullShelters++;}fclose(fp);}
    fp=fopen("resources.dat","rb");if(fp){while(fread(&r,sizeof(r),1,fp))resources+=r.quantity;fclose(fp);}
    fp=fopen("volunteers.dat","rb");if(fp){while(fread(&vol,sizeof(vol),1,fp))volunteers++;fclose(fp);}
    char buf[1024];
    sprintf(buf,
        "========== SYSTEM DASHBOARD ==========\n\n"
        "  Total Victims          : %d\n"
        "  Total Volunteers       : %d\n"
        "  Total Shelters         : %d\n"
        "  Full Shelters          : %d\n"
        "  Total Shelter Capacity : %d\n"
        "  Occupied Spots         : %d\n"
        "  Available Spots        : %d\n"
        "  Total Resource Units   : %d\n"
        "  Highest Injury Level   : %d\n",
        victims,volunteers,shelters,fullShelters,totalCap,occupied,totalCap-occupied,resources,maxInj==-1?0:maxInj);
    GtkTextBuffer *tbuf=gtk_text_view_get_buffer(GTK_TEXT_VIEW(summary_view));
    gtk_text_buffer_set_text(tbuf,buf,-1);
    set_status("Dashboard refreshed.");
}

/* ============================
   TAB SWITCH
   ============================ */
void switch_tab(GtkWidget *btn, gpointer name) {
    gtk_stack_set_visible_child_name(GTK_STACK(stack),(const char*)name);
    if(strcmp((char*)name,"victim")==0) refresh_victim_list();
    else if(strcmp((char*)name,"shelter")==0) refresh_shelter_list();
    else if(strcmp((char*)name,"resource")==0) refresh_resource_list();
    else if(strcmp((char*)name,"volunteer")==0) refresh_volunteer_list();
    else if(strcmp((char*)name,"distribution")==0){refresh_distribution_list();refresh_resource_list();}
    else if(strcmp((char*)name,"summary")==0) on_refresh_summary(NULL,NULL);
}

/* ============================
   PAGE BUILDERS
   ============================ */
GtkWidget* build_victim_page() {
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox),12);
    GtkWidget *hform=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    v_id=make_entry("Victim ID (e.g. V001)");
    v_name=make_entry("Full Name");
    v_age=make_entry("Age");
    v_injury=make_entry("Injury Level (1-10)");
    v_location=make_entry("Location");
    gtk_box_pack_start(GTK_BOX(hform),v_id,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),v_name,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),v_age,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),v_injury,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),v_location,TRUE,TRUE,0);
    GtkWidget *hbtn=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Add",G_CALLBACK(on_add_victim),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Update",G_CALLBACK(on_update_victim),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Delete",G_CALLBACK(on_delete_victim),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Search",G_CALLBACK(on_search_victim),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Show All",G_CALLBACK((GCallback)refresh_victim_list),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Sort by Injury",G_CALLBACK(on_sort_victims),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hform,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hbtn,FALSE,FALSE,0);
    GtkListStore *store;
    const char *cols[]={"ID","Name","Age","Injury","Location","Shelter ID"};
    v_list=make_treeview(&store,6,cols);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll),v_list);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);
    return vbox;
}

GtkWidget* build_shelter_page() {
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox),12);
    GtkWidget *hform=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    s_id=make_entry("Shelter ID (e.g. S001)");
    s_name=make_entry("Shelter Name");
    s_location=make_entry("Location");
    s_capacity=make_entry("Capacity");
    gtk_box_pack_start(GTK_BOX(hform),s_id,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),s_name,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),s_location,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),s_capacity,TRUE,TRUE,0);
    GtkWidget *hbtn=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Add",G_CALLBACK(on_add_shelter),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Update Capacity",G_CALLBACK(on_update_shelter),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Delete",G_CALLBACK(on_delete_shelter),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Search",G_CALLBACK(on_search_shelter),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Show All",G_CALLBACK((GCallback)refresh_shelter_list),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hform,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hbtn,FALSE,FALSE,0);
    GtkListStore *store;
    const char *cols[]={"ID","Name","Location","Capacity","Current","Available","Status"};
    s_list=make_treeview(&store,7,cols);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll),s_list);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);
    return vbox;
}

GtkWidget* build_resource_page() {
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox),12);
    GtkWidget *hform=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    r_id=make_entry("Resource ID (e.g. R001)");
    r_name=make_entry("Resource Name");
    r_type=make_entry("Type (Food/Water/Medicine)");
    r_quantity=make_entry("Quantity");
    gtk_box_pack_start(GTK_BOX(hform),r_id,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),r_name,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),r_type,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),r_quantity,TRUE,TRUE,0);
    GtkWidget *hbtn=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Add",G_CALLBACK(on_add_resource),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Update Quantity",G_CALLBACK(on_update_resource),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Delete",G_CALLBACK(on_delete_resource),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Search",G_CALLBACK(on_search_resource),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Show All",G_CALLBACK((GCallback)refresh_resource_list),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hform,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hbtn,FALSE,FALSE,0);
    GtkListStore *store;
    const char *cols[]={"ID","Name","Type","Quantity"};
    r_list=make_treeview(&store,4,cols);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll),r_list);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);
    return vbox;
}

GtkWidget* build_volunteer_page() {
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox),12);
    GtkWidget *hform=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    vol_id=make_entry("Volunteer ID (e.g. VOL001)");
    vol_name=make_entry("Full Name");
    vol_role=make_entry("Role: Medical / Food / Transport");
    gtk_box_pack_start(GTK_BOX(hform),vol_id,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),vol_name,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),vol_role,TRUE,TRUE,0);
    GtkWidget *hbtn=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Add",G_CALLBACK(on_add_volunteer),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Update Role",G_CALLBACK(on_update_volunteer),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Delete",G_CALLBACK(on_delete_volunteer),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Search",G_CALLBACK(on_search_volunteer),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Show All",G_CALLBACK((GCallback)refresh_volunteer_list),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hform,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hbtn,FALSE,FALSE,0);
    GtkListStore *store;
    const char *cols[]={"ID","Name","Role"};
    vol_list=make_treeview(&store,3,cols);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll),vol_list);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);
    return vbox;
}

GtkWidget* build_distribution_page() {
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox),12);
    GtkWidget *hform=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    d_victimid=make_entry("Victim ID");
    d_resourceid=make_entry("Resource ID");
    d_quantity=make_entry("Quantity");
    gtk_box_pack_start(GTK_BOX(hform),d_victimid,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),d_resourceid,TRUE,TRUE,0);
    gtk_box_pack_start(GTK_BOX(hform),d_quantity,TRUE,TRUE,0);
    GtkWidget *hbtn=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,6);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Distribute Resource",G_CALLBACK(on_assign_resource),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(hbtn),make_button("Show All Records",G_CALLBACK((GCallback)refresh_distribution_list),NULL),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hform,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hbtn,FALSE,FALSE,0);
    GtkListStore *store;
    const char *cols[]={"Victim ID","Resource ID","Quantity"};
    d_list=make_treeview(&store,3,cols);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll),GTK_POLICY_AUTOMATIC,GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(scroll),d_list);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);
    return vbox;
}

GtkWidget* build_summary_page() {
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,8);
    gtk_container_set_border_width(GTK_CONTAINER(vbox),12);
    gtk_box_pack_start(GTK_BOX(vbox),make_button("Refresh Dashboard",G_CALLBACK(on_refresh_summary),NULL),FALSE,FALSE,0);
    summary_view=gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(summary_view),FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(summary_view),TRUE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(summary_view),16);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(summary_view),12);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL,NULL);
    gtk_container_add(GTK_CONTAINER(scroll),summary_view);
    gtk_widget_set_vexpand(scroll,TRUE);
    gtk_box_pack_start(GTK_BOX(vbox),scroll,TRUE,TRUE,0);
    return vbox;
}

/* ============================
   MAIN
   ============================ */
int main(int argc, char *argv[]) {
    gtk_init(&argc,&argv);
    main_window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window),"Disaster Relief Management System");
    gtk_window_set_default_size(GTK_WINDOW(main_window),1000,600);
    g_signal_connect(main_window,"destroy",G_CALLBACK(gtk_main_quit),NULL);

    GtkWidget *root=gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_container_add(GTK_CONTAINER(main_window),root);

    GtkWidget *navbar=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,4);
    gtk_container_set_border_width(GTK_CONTAINER(navbar),8);
    GtkWidget *lbl=gtk_label_new("  DRMS  ");
    gtk_widget_set_margin_end(lbl,8);
    gtk_box_pack_start(GTK_BOX(navbar),lbl,FALSE,FALSE,0);

    const char *tabs[][2]={
        {"Victims","victim"},{"Shelters","shelter"},
        {"Resources","resource"},{"Volunteers","volunteer"},
        {"Distribution","distribution"},{"Dashboard","summary"}
    };
    for(int i=0;i<6;i++){
        GtkWidget *b=make_button(tabs[i][0],G_CALLBACK(switch_tab),(gpointer)tabs[i][1]);
        gtk_box_pack_start(GTK_BOX(navbar),b,FALSE,FALSE,0);
    }

    gtk_box_pack_start(GTK_BOX(root),navbar,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(root),gtk_separator_new(GTK_ORIENTATION_HORIZONTAL),FALSE,FALSE,0);

    stack=gtk_stack_new();
    gtk_stack_add_named(GTK_STACK(stack),build_victim_page(),"victim");
    gtk_stack_add_named(GTK_STACK(stack),build_shelter_page(),"shelter");
    gtk_stack_add_named(GTK_STACK(stack),build_resource_page(),"resource");
    gtk_stack_add_named(GTK_STACK(stack),build_volunteer_page(),"volunteer");
    gtk_stack_add_named(GTK_STACK(stack),build_distribution_page(),"distribution");
    gtk_stack_add_named(GTK_STACK(stack),build_summary_page(),"summary");
    gtk_stack_set_visible_child_name(GTK_STACK(stack),"victim");
    gtk_widget_set_vexpand(stack,TRUE);
    gtk_box_pack_start(GTK_BOX(root),stack,TRUE,TRUE,0);

    status_bar=gtk_statusbar_new();
    gtk_box_pack_end(GTK_BOX(root),status_bar,FALSE,FALSE,0);
    set_status("Welcome to Disaster Relief Management System");

    refresh_victim_list();
    gtk_widget_show_all(main_window);
    gtk_main();
    return 0;
}