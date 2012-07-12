/* list helper stuff goes here */

/* UnlinkListItem(item,list) */
#define UnlinkListItem(x,y) { if(x->prev==NULL && x->next==NULL) { y=NULL ;} else {if(x->next==NULL) {x->prev->next=NULL ;} else { if(x->prev==NULL) { x->next->prev=NULL ;y=x->next ;} else { x->next->prev=x->prev ; x->prev->next=x->next ; } } } }

#define FindEndOfList(x) {while(x->next!=NULL){x=x->next;}}

/* LinkToList list (item, tempvar, list) */
#define LinkToList(x,y,z) { y=z;x->prev=NULL; x->next=NULL; if(z==NULL) { z=x;y=x;} else { FindEndOfList(y) ; y->next=x;x->prev=y;y=x;}}
