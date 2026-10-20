#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#define MAX_CHARS 256


void countFrequency(const char *filename,int freq[],int *totalChars){
    *totalChars=0;
    for(int i=0;i<MAX_CHARS;i++)
    { 
        freq[i]=0;
    }
    FILE *file=fopen(filename,"r");
    if(file==NULL)
    {
        printf("文件打开错误\n");
        return;
    }
    int ch=0;
    while((ch=fgetc(file))!=EOF)
    {
        freq[(unsigned char)ch]++;
        (*totalChars)++;
    }
    fclose(file);
}

typedef struct Huffmannode{
    char character;
    int frequency;
    int x;
    int y;
    struct Huffmannode *Left;
    struct Huffmannode *Right;
}Huffmannode;

Huffmannode* createnode(char character,int frequency){
    Huffmannode *node=(Huffmannode*)malloc(sizeof(Huffmannode));
    node->character=character;
    node->frequency=frequency;
    node->Left=NULL;
    node->Right=NULL;
    node->x=0;
    node->y=0;
    return node;
}

Huffmannode* buildHuffmantree(int freq[]){
    Huffmannode *nodes[MAX_CHARS];
    int num=0;
    for(int i=0;i<MAX_CHARS;i++){
        if(freq[i]>0){
            nodes[num]=createnode((char)i,freq[i]);
            num++;
        }
    }
    while(num>1){
        int min1=0,min2=-1;
        for(int i=1;i<num;i++){
            if(nodes[i]->frequency<nodes[min1]->frequency){
                min1=i;
            }
        for (int i = 0; i < num; i++) {
        if (i == min1) continue;
        if (min2 == -1 || nodes[i]->frequency < nodes[min2]->frequency) min2 = i;
    }
    Huffmannode *parent = createnode('\0',
        nodes[min1]->frequency + nodes[min2]->frequency);
    if (!parent) { /* 释放 nodes[0..num-1] 后 return NULL */ }
    parent->Left  = nodes[min1];
    parent->Right = nodes[min2];
    nodes[min1] = parent;
    nodes[min2] = nodes[num - 1];
    num--;
}
    }
    if(num==0){return NULL;}

    return nodes[0];
}

int getdepth(Huffmannode *root){
    if(root==NULL)return 0;

    int leftdepth=getdepth(root->Left);
    int rightdepth=getdepth(root->Right);
    return (leftdepth>rightdepth?leftdepth:rightdepth)+1;
}

void getwidth(Huffmannode *root,int depth,int* col){
    if(root==NULL)return;
    getwidth(root->Left,depth+1,col);
    root->x=*col;
    root->y=depth;
    *col+=5;
    getwidth(root->Right,depth+1,col);
}

void draw(Huffmannode* root,char huabu[512][1500]){
    char charater[10];
    if(root==NULL)return;
    if(root->Left==NULL&&root->Right==NULL){
        sprintf(charater,"%c%d",root->character,root->frequency);
    }
    else{
        sprintf(charater,"%d",root->frequency);
    }
    for(int i=0;charater[i]!='\0';i++)
    {
         huabu[root->y*2][root->x+i]=charater[i];
        
    }
     if(root->Left){
            huabu[root->y*2+1][root->x-1]='/';
            draw(root->Left,huabu);
        }
    if(root->Right){
            huabu[root->y*2+1][root->x+1]='\\';
            draw(root->Right,huabu);
        }
    
}

void printtree(Huffmannode* root){
    if(root==NULL)return;
    static char huabu[512][1500];
    int width=0;int depth=getdepth(root);
    getwidth(root,0,&width);
    width+=5;
    depth=depth*2;
    for(int i=0;i<depth;i++){
        for(int j=0;j<width;j++){
            huabu[i][j]=' ';
        }
    }
    draw(root,huabu);
    for(int i=0;i<depth;i++){
        int thelast=width-1;
        while(thelast>=0&&huabu[i][thelast]!=' '){
            thelast--;
        }
        for(int j=0;j<=depth;j++){
            printf("%c",huabu[i][j]);
        }
        printf("\n");
    }
}

void createtable(Huffmannode* root,char temporarary[],int depth,char codetable[][MAX_CHARS]){
    if(root==NULL)return;
    if(root ->Left==NULL&&root->Right==NULL){
        temporarary[depth]='\0';
        strcpy(codetable[(unsigned char)root->character],temporarary);
        return;
    }

    //向左走
    temporarary[depth]='0';
    createtable(root->Left,temporarary,depth+1,codetable);

    //向右走
    temporarary[depth]='1';
    createtable(root->Right,temporarary,depth+1,codetable);

}

int putoutfile(const char* inputFile,const char* outFile,int totalChars,char codetable[][MAX_CHARS]){
    FILE* in=fopen(inputFile,"r");
    FILE* out=fopen(outFile,"wb");

    if(in==NULL){
        printf("编码失败，源文件打不开");
        return 0;
    }
    if(out==NULL){
        printf("编码失败，输出文件创建不了");
        fclose(in);
        return 0;
    }

    fwrite(&totalChars,sizeof(int),1,out);
    unsigned char buffer=0;
    int bitcount=0;
    int ch=0,encoded=0;

    while((ch=fgetc(in))!=EOF)
    {
        const char* code=codetable[(unsigned char)ch];
        if(code[0]=='\0'){                      /* 新：空编码保护 */
            printf("警告：字符 0x%02X 没有编码，已跳过\n",(unsigned char)ch);
            continue;
        }
        for(int i=0;code[i]!='\0';i++){
            if(code[i]=='1') buffer |=(unsigned char)(1u<<(7-bitcount));
            if(++bitcount==8){
                fwrite(&buffer,1,1,out);
                buffer=0;
                bitcount=0;
            }
        }
        encoded++;
    }
    if(bitcount!=0){
        fwrite(&buffer,1,1,out);
    }
    fclose(in);fclose(out);
    if(encoded!=totalChars)
        printf("警告：头部记录 %d 字符，实际编码 %d 字符\n",totalChars,encoded);
    return 1;
}

void printcodetable(char codetable[][MAX_CHARS]){
    printf("            哈夫曼编码表            \n");
    printf("\n");
    for(int i=0;i<MAX_CHARS;i++){
        if(codetable[i][0]!='\0'){
            if(i==' '){
                printf("%-12s %-20s\n","(空格)",codetable[i]);
            }else if(i=='\n'){
                printf("%-12s %-20s\n","(换行)",codetable[i]);
            }else if(i=='\t'){
                printf("%-12s %-20s\n","(制表)",codetable[i]);
            }
            else{
                printf(" %c             %-20s\n",(char)i,codetable[i]);
            }
        }
    }
}

void savecodetable(const char* filename,char codetable[][MAX_CHARS]){
    FILE* out=fopen(filename,"w");
    if(out==NULL)return;

    fprintf(out,"            哈夫曼编码表\n");
    fprintf(out,"字符   编码\n\n");
    for(int i=0;i<MAX_CHARS;i++){
        if(codetable[i][0]!='\n'){
            if(i==' '){
                fprintf(out,"空格   %s\n",codetable[i]);
            }
            else if(i=='\n'){
                fprintf(out,"换行   %s\n",codetable[i]);
            }
            else{fprintf(out,"%c    %s\n",(char)i,codetable[i]);}
        }
    }
    fprintf(out,"data start\n");
    for(int i=0;i<MAX_CHARS;i++){
        if(codetable[i][0]!='\0'){
            fprintf(out,"%d   %s",i,codetable[i]);
        }
    }
}

Huffmannode* treefromtable(const char* filename){
    FILE* file=fopen(filename,"r");
    if(file==NULL)return NULL;

    Huffmannode* root=createnode('\0',0);     
    int isfind=0;
    char line[300];
    while(fgets(line,sizeof(line),file)){
        if(strstr(line,"data start\n")!=NULL){
            isfind=1;
            continue;
        }
        if(isfind){
            int asc=0;
            char code[MAX_CHARS];
            Huffmannode* current=root;
            if(sscanf(line,"%d   %s",&asc,code)==2){
                for(int i=0;code[i]!='\0';i++){
                    //left
                    if(code[i]=='0'){
                        if(current->Left==NULL){
                            current->Left=createnode('0',0);
                            }
                        current=current->Left;
                    }else if(code[i]=='1'){
                        if(current->Right==NULL){
                            current->Right=createnode('0',0);
                            }
                        current=current->Right;
                    }
                }
                current->character=(char)asc;
            }
        }
    }
    fclose(file);
    return root;
}

void decode(const char* codetable,const char* codeart,const char* putout){
    Huffmannode* root=treefromtable(codetable);
    if(root==NULL){
        printf("编码表文件读取失败\n");
        return;
    }
    FILE* in=fopen(codeart,"rb");
    FILE* out=fopen(putout,"w");
    if(in==NULL||out==NULL){
        printf("编码文件打开失败\n");
        return;
    }
    int totalchars=0;
    fread(&totalchars,sizeof(int),1,in);

    Huffmannode* current=root;
    unsigned char buffer;
    int decodecount=0;

    while(fread(&buffer,1,1,in)==1&&decodecount<totalchars){
        for(int i=7;i>=0;i--){
            int bit=(buffer>>i)&1;
            current=(bit==0)?current->Left:current->Right;
            if(current==NULL){
                printf("解码异常");
                fclose(in);
                fclose(out);
                return;
            }
            if(current->Left==NULL&&current->Right==NULL){
                fputc(current->character,out);
                decodecount++;
                current==root;
            }
            if(decodecount==totalchars)break;
        }
        
    }

    fclose(in);
    fclose(out);
    printf("解码成功，结果%s\n",putout);
}

void freetree(Huffmannode* root){
    if(root==NULL)return;

    freetree(root->Left);
    freetree(root->Right);
    free(root);

}



int main(){
    int freq[MAX_CHARS]={0};
    int totalchars=0;
    Huffmannode* root=NULL;
    int choice=0;
    char codetable[MAX_CHARS][MAX_CHARS];
    char temporarary[MAX_CHARS];
    char filename[100];


    int isload=0;
    int istree=0;
    int isfile=0;

    while(1){
        printf("            哈夫曼编码\n");
        printf("\n");
        printf("1.读取文章统计字符频率\n");
        printf("2.根据频率建树\n");
        printf("3.打印树\n");
        printf("4.生成编码表和编码文件,打印编码表\n");
        printf("5.解码文件\n");
        printf("6.退出\n");
        printf("\n");
        printf("请选择:");


        if(scanf("%d",&choice)!=1){
            printf("无效输入\n");
            while(getchar()!='\n'&&!feof(stdin));
            continue;
        }
        while(getchar()!='\n'&&!feof(stdin));


        switch(choice){
            case 1:{
                
                printf("\n请输入文件名包括后缀:");
                scanf("%s",filename);

                if(root!=NULL)freetree(root);

                countFrequency(filename,freq,totalchars);
                if(totalchars>0){
                    isload=1;
                    istree=0;
                    isfile=0;
                }
                break;
            }
            case 2:{
                if(isload==0){
                    printf("请先执行步骤1\n");
                    break;
                }
                root=buildHuffmantree(freq);
                if(root!=NULL){
                    istree=1;
                }
                printf("哈夫曼树构建完成\n");
                break;
            }
            case 3:{
                if(istree==0){
                    printf("哈夫曼树未构建\n");
                    break;
                }
                printtree(root);
                break;
            }
            case 4:{
                if(istree==0){
                    printf("哈夫曼树未构建\n");
                    break;
                }
                memset(temporarary,0,sizeof(temporarary));
                memset(codetable,0,sizeof(codetable));

                createtable(root,temporarary,0,codetable);
                
                char outfile[100];char outfile2[100];
                printf("请输入生成的文件名（英文）");
                scanf("%s",outfile);
                putoutfile(filename,outfile,totalchars,codetable);
                printf("请输入生成的编码表文件名（英文）");
                scanf("%s",outfile2);
                savecodetable(outfile2,codetable);
                printcodetable(codetable);
                break;
            }
            case 5:{
                char table[100];
                char file[100];
                char outfile[100];
                printf("请输入编码表文件名:");
                scanf("%s",table);
                printf("请输入编码文件名:");
                scanf("%s",file);
                printf("请输入输出文件名:");
                scanf("%s",outfile);
                decode(table,file,outfile);
                break;
            }
            case 6:{
                freetree(root);
                return 0;
            }
            default:{
                printf("无效的选择，请选其他\n");
                break;
            }

        }
        
    }
    return 0;
}