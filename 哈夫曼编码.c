#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif

#define MAX_CHARS 256

#define HUABU_H 512
#define HUABU_W 4096

#define NAME_MAX_LEN 512

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

    if(node==NULL)return NULL;
    node->character=character;
    node->frequency=frequency;
    node->Left=NULL;
    node->Right=NULL;
    node->x=0;
    node->y=0;
    return node;
}

void freetree(Huffmannode* root){
    if(root==NULL)return;

    freetree(root->Left);
    freetree(root->Right);
    free(root);

}

int countFrequency(const char *filename,int freq[],int *totalChars){
    *totalChars=0;
    for(int i=0;i<MAX_CHARS;i++)
    {
        freq[i]=0;
    }

    FILE *file=fopen(filename,"rb");

    if(file==NULL)
    {
        printf("文件打开错误：%s\n",filename);
        return 0;
    }
    int ch=0;
    while((ch=fgetc(file))!=EOF)
    {
        freq[(unsigned char)ch]++;
        (*totalChars)++;
    }
    fclose(file);
    return 1;
}

Huffmannode* buildHuffmantree(int freq[]){
    Huffmannode *nodes[MAX_CHARS];
    int num=0;
    for(int i=0;i<MAX_CHARS;i++){
        if(freq[i]>0){
            nodes[num]=createnode((char)i,freq[i]);

            if(nodes[num]==NULL){
                for(int j=0;j<num;j++)freetree(nodes[j]);
                return NULL;
            }
            num++;
        }
    }
    while(num>1){

        int min1=0,min2=-1;
        for(int i=1;i<num;i++)
            if(nodes[i]->frequency<nodes[min1]->frequency)min1=i;
        for(int i=0;i<num;i++){
            if(i==min1)continue;
            if(min2==-1||nodes[i]->frequency<nodes[min2]->frequency)min2=i;
        }
        Huffmannode *parent=createnode('\0',nodes[min1]->frequency+nodes[min2]->frequency);

        if(parent==NULL){
            for(int j=0;j<num;j++)freetree(nodes[j]);
            return NULL;
        }
        parent->Left=nodes[min1];
        parent->Right=nodes[min2];
        nodes[min1]=parent;
        nodes[min2]=nodes[num-1];
        num--;
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

void getwidth(Huffmannode *root,int depth,int* col,int step){

    if(root==NULL)return;

    root->y=depth;
    if(root->Left==NULL&&root->Right==NULL){
        root->x=*col;
        *col+=step;
        return;
    }
    getwidth(root->Left,depth+1,col,step);
    getwidth(root->Right,depth+1,col,step);
    if(root->Left&&root->Right)root->x=(root->Left->x+root->Right->x)/2;
    else if(root->Left)root->x=root->Left->x;
    else root->x=root->Right->x;
}

static char printableChar(char c){
    unsigned char u=(unsigned char)c;
    if(u<32||u==127)return '?';
    return (char)u;
}

static int labelText(const Huffmannode *root,char *buf,int size){
    int n;
    if(root->Left==NULL&&root->Right==NULL)
        n=snprintf(buf,size,"%c%d",printableChar(root->character),root->frequency);
    else
        n=snprintf(buf,size,"%d",root->frequency);
    if(n<0)n=1;
    if(n>=size)n=size-1;
    return n;
}

static int maxLabelWidth(Huffmannode *root){
    if(root==NULL)return 1;
    char buf[24];
    int self=labelText(root,buf,sizeof(buf));
    int l=maxLabelWidth(root->Left);
    int r=maxLabelWidth(root->Right);
    int m=l>r?l:r;
    return self>m?self:m;
}

void draw(Huffmannode* root,char huabu[HUABU_H][HUABU_W]){
    char charater[24];
    if(root==NULL)return;
    labelText(root,charater,sizeof(charater));
    int row=root->y*2;

    {
        int len=(int)strlen(charater);
        int start=root->x-len/2;
        if(start<0)start=0;
        if(row>=0&&row<HUABU_H){
            for(int i=0;i<len;i++)
            {
                int col=start+i;
                if(col>=0&&col<HUABU_W)huabu[row][col]=charater[i];
            }
        }
    }
    if(root->Left){
        int r=row+1,c=(root->x+root->Left->x)/2;
        if(r<HUABU_H&&c>=0&&c<HUABU_W)huabu[r][c]='/';
        draw(root->Left,huabu);
    }
    if(root->Right){
        int r=row+1,c=(root->x+root->Right->x+1)/2;
        if(r<HUABU_H&&c>=0&&c<HUABU_W)huabu[r][c]='\\';
        draw(root->Right,huabu);
    }
}

static void printIndentNode(Huffmannode *node,const char *prefix,int isLast){
    if(node==NULL)return;
    char label[24];
    labelText(node,label,sizeof(label));
    printf("%s%s%s\n",prefix,isLast?"\\-- ":"|-- ",label);
    char next[300];
    snprintf(next,sizeof(next),"%s%s",prefix,isLast?"    ":"|   ");
    if(node->Left)printIndentNode(node->Left,next,node->Right==NULL);
    if(node->Right)printIndentNode(node->Right,next,1);
}

void printtree(Huffmannode* root){
    if(root==NULL)return;
    static char huabu[HUABU_H][HUABU_W];
    int width=0;
    int treeDepth=getdepth(root);

    int step=maxLabelWidth(root)+2;
    if(step<4)step=4;
    getwidth(root,0,&width,step);

    if(width>110){
        char rootLabel[24];
        labelText(root,rootLabel,sizeof(rootLabel));
        printf("（树较宽，改用缩进形式显示）\n");
        printf("%s\n",rootLabel);
        if(root->Left)printIndentNode(root->Left,"",root->Right==NULL);
        if(root->Right)printIndentNode(root->Right,"",1);
        return;
    }
    int rows=treeDepth*2;
    if(rows>HUABU_H)rows=HUABU_H;

    memset(huabu,' ',sizeof(huabu));
    draw(root,huabu);
    for(int i=0;i<rows;i++){

        int thelast=HUABU_W-1;
        while(thelast>=0&&huabu[i][thelast]==' ')thelast--;

        for(int j=0;j<=thelast;j++){
            printf("%c",huabu[i][j]);
        }
        printf("\n");
    }
}

void createtable(Huffmannode* root,char temporarary[],int depth,char codetable[][MAX_CHARS]){
    if(root==NULL)return;
    if(root ->Left==NULL&&root->Right==NULL){

        if(depth==0){
            temporarary[0]='0';
            temporarary[1]='\0';
        }else{
            temporarary[depth]='\0';
        }
        strcpy(codetable[(unsigned char)root->character],temporarary);
        return;
    }

    temporarary[depth]='0';
    createtable(root->Left,temporarary,depth+1,codetable);

    temporarary[depth]='1';
    createtable(root->Right,temporarary,depth+1,codetable);

}

int putoutfile(const char* inputFile,const char* outFile,int totalChars,char codetable[][MAX_CHARS]){

    FILE* in=fopen(inputFile,"rb");
    FILE* out=fopen(outFile,"wb");

    if(in==NULL){
        printf("编码失败：源文件 %s 打不开\n",inputFile);
        return 0;
    }
    if(out==NULL){
        printf("编码失败：输出文件 %s 无法创建\n",outFile);
        fclose(in);
        return 0;
    }

    fwrite(&totalChars,sizeof(int),1,out);
    unsigned char buffer=0;
    int bitcount=0;
    int ch=0;
    int encoded=0;

    while((ch=fgetc(in))!=EOF)
    {
        const char* code=codetable[(unsigned char)ch];

        if(code[0]=='\0'){
            printf("警告：字符 0x%02X 没有编码，已跳过\n",(unsigned char)ch);
            continue;
        }
        for(int i=0;code[i]!='\0';i++){
            if(code[i]=='1'){
                buffer|=(unsigned char)(1u<<(7-bitcount));
            }
            bitcount++;

            if(bitcount==8){
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

    fclose(in);
    fclose(out);
    if(encoded!=totalChars){
        printf("警告：头部记录 %d 字节，实际编码 %d 字节（码表与源文件不一致）\n",
               totalChars,encoded);
        return 0;
    }
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
            }else if(i=='\r'){
                printf("%-12s %-20s\n","(回车)",codetable[i]);
            }
            else{
                printf(" %c             %-20s\n",(char)i,codetable[i]);
            }
        }
    }
}

static void putReadableChar(FILE* out,int i){
    switch(i){
        case ' ':  fprintf(out,"空格"); break;
        case '\n': fprintf(out,"换行"); break;
        case '\t': fprintf(out,"制表"); break;
        case '\r': fprintf(out,"回车"); break;
        default:
            if(i>=32&&i<127) fprintf(out,"%c",(char)i);
            else             fprintf(out,"\\x%02X",i);
            break;
    }
}

int savecodetable(const char* filename,char codetable[][MAX_CHARS]){
    FILE* out=fopen(filename,"w");
    if(out==NULL){
        printf("编码表文件无法创建：%s\n",filename);
        return 0;
    }

    fprintf(out,"            哈夫曼编码表\n");
    fprintf(out,"字符   编码\n\n");
    for(int i=0;i<MAX_CHARS;i++){

        if(codetable[i][0]!='\0'){

            putReadableChar(out,i);
            fprintf(out,"   %s\n",codetable[i]);
        }
    }
    fprintf(out,"data start\n");
    for(int i=0;i<MAX_CHARS;i++){
        if(codetable[i][0]!='\0'){

            fprintf(out,"%d   %s\n",i,codetable[i]);
        }
    }

    fclose(out);
    return 1;
}

Huffmannode* treefromtable(const char* filename){
    FILE* file=fopen(filename,"r");
    if(file==NULL)return NULL;

    Huffmannode* root=createnode('\0',0);
    if(root==NULL){fclose(file);return NULL;}
    int isfind=0;
    int leaves=0;
    char line[300];

    while(fgets(line,sizeof(line),file)!=NULL){

        if(strstr(line,"data start")!=NULL){
            isfind=1;
            continue;
        }
        if(isfind){
            int asc=0;
            char code[MAX_CHARS];
            Huffmannode* current=root;

            if(sscanf(line,"%d %255s",&asc,code)==2){
                for(int i=0;code[i]!='\0';i++){

                    if(code[i]=='0'){
                        if(current->Left==NULL){

                            current->Left=createnode('\0',0);
                            if(current->Left==NULL){
                                freetree(root);fclose(file);return NULL;
                            }
                        }
                        current=current->Left;
                    }else if(code[i]=='1'){
                        if(current->Right==NULL){
                            current->Right=createnode('\0',0);
                            if(current->Right==NULL){
                                freetree(root);fclose(file);return NULL;
                            }
                        }
                        current=current->Right;
                    }
                }
                current->character=(char)asc;
                leaves++;
            }
        }
    }
    fclose(file);

    if(leaves==0){
        freetree(root);
        return NULL;
    }
    return root;
}

int decode(const char* codetableFile,const char* codeart,const char* putout){
    Huffmannode* root=treefromtable(codetableFile);
    if(root==NULL){
        printf("编码表文件读取失败或内容为空：%s\n",codetableFile);
        return 0;
    }
    FILE* in=fopen(codeart,"rb");
    if(in==NULL){
        printf("编码文件打开失败：%s\n",codeart);
        freetree(root);
        return 0;
    }
    FILE* out=fopen(putout,"wb");
    if(out==NULL){
        printf("输出文件无法创建：%s\n",putout);
        fclose(in);
        freetree(root);
        return 0;
    }
    int totalchars=0;
    if(fread(&totalchars,sizeof(int),1,in)!=1||totalchars<=0){
        printf("编码文件头部损坏或为空\n");
        fclose(in);
        fclose(out);
        freetree(root);
        return 0;
    }

    Huffmannode* current=root;
    unsigned char buffer;
    int decodecount=0;

    while(decodecount<totalchars&&fread(&buffer,1,1,in)==1){
        for(int i=7;i>=0;i--){
            int bit=(buffer>>i)&1;
            current=(bit==0)?current->Left:current->Right;
            if(current==NULL){
                printf("解码异常：比特流与编码表不匹配\n");
                fclose(in);
                fclose(out);
                freetree(root);
                return 0;
            }
            if(current->Left==NULL&&current->Right==NULL){
                fputc(current->character,out);
                decodecount++;

                current=root;
            }
            if(decodecount==totalchars)break;
        }

    }

    fclose(in);
    fclose(out);

    freetree(root);
    printf("解码成功，共 %d 字符，结果 %s\n",decodecount,putout);
    return 1;
}

static long fileSize(const char* path){
    FILE* file=fopen(path,"rb");
    if(file==NULL)return -1;

    if(fseek(file,0,SEEK_END)!=0){
        fclose(file);
        return -1;
    }
    long size=ftell(file);
    fclose(file);
    return size;
}

static void printcompressionratio(const char* codefile,const char* tablefile,
                                  int totalchars,int freq[],char codetable[][MAX_CHARS]){
    long bin=fileSize(codefile);

    if(bin<0){
        printf("找不到压缩文件：%s\n",codefile);
        return;
    }
    long tbl=fileSize(tablefile);

    if(tbl<0)tbl=0;

    long bits=0;

    for(int i=0;i<MAX_CHARS;i++){
        if(codetable[i][0]!='\0')bits+=(long)freq[i]*(long)strlen(codetable[i]);
    }
    long databytes=bin-(long)sizeof(int);

    if(databytes<0)databytes=0;

    printf("\n            压缩率统计            \n\n");
    printf("原文大小      : %d 字节\n",totalchars);
    printf("压缩文件      : %ld 字节（含 %d 字节头部）\n",bin,(int)sizeof(int));
    printf("码表文件      : %ld 字节\n",tbl);
    printf("编码总位数    : %ld bit\n",bits);
    printf("比特流字节数  : %ld 字节（末尾不足 8 bit 补零）\n",databytes);
    printf("平均码长      : %.2f bit/字符\n",totalchars>0?(double)bits/totalchars:0.0);
    printf("\n");
    printf("压缩率(体积)  : %.2f%%   压缩文件 / 原文\n",
           totalchars>0?100.0*(double)bin/totalchars:0.0);
    printf("节省率        : %.2f%%   1 - 压缩文件 / 原文\n",
           totalchars>0?100.0*(1.0-(double)bin/totalchars):0.0);
    printf("含码表总占用  : %.2f%%   (压缩文件 + 码表) / 原文\n",
           totalchars>0?100.0*(double)(bin+tbl)/totalchars:0.0);
    if(totalchars>0&&(double)bin/totalchars>0.9)
        printf("提示：原文件较小，码表开销占比大，文件越大越接近第一种压缩率\n");
}

static int readLine(char *buf,int size){
    if(fgets(buf,size,stdin)==NULL)return 0;
    int n=(int)strlen(buf);
    while(n>0&&(buf[n-1]=='\n'||buf[n-1]=='\r'))buf[--n]='\0';
    return 1;
}

int main(){

#ifdef _WIN32
    SetConsoleOutputCP(65001);
#endif
    int freq[MAX_CHARS]={0};
    int totalchars=0;
    Huffmannode* root=NULL;
    int choice=0;

    static char codetable[MAX_CHARS][MAX_CHARS];
    char temporarary[MAX_CHARS];
    char filename[NAME_MAX_LEN];
    char codefile[NAME_MAX_LEN]={0};
    char tablefile[NAME_MAX_LEN]={0};

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
        printf("6.计算压缩率\n");
        printf("7.退出\n");

        printf("\n");
        printf("请选择:");

        char line[64];
        if(!readLine(line,sizeof(line)))break;
        if(sscanf(line,"%d",&choice)!=1){
            printf("无效输入，请输入数字选项\n");
            continue;
        }

        switch(choice){
            case 1:{

                printf("\n请输入文件名包括后缀:");
                if(!readLine(filename,sizeof(filename))||filename[0]=='\0'){
                    printf("文件名无效\n");
                    break;
                }

                if(root!=NULL){freetree(root);root=NULL;}

                if(!countFrequency(filename,freq,&totalchars)){
                    isload=0;
                    istree=0;
                    isfile=0;
                    break;
                }
                if(totalchars>0){
                    isload=1;
                    istree=0;
                    isfile=0;
                    printf("统计完成：共 %d 字节\n",totalchars);
                }else{
                    isload=0;
                    istree=0;
                    isfile=0;
                    printf("文件为空，无法建树\n");
                }
                break;
            }
            case 2:{
                if(isload==0){
                    printf("请先执行步骤1\n");
                    break;
                }

                if(root!=NULL){freetree(root);root=NULL;istree=0;}
                root=buildHuffmantree(freq);
                if(root!=NULL){
                    istree=1;

                    printf("哈夫曼树构建完成\n");
                }else{
                    istree=0;
                    printf("哈夫曼树构建失败（频次表为空或内存不足）\n");
                }
                break;
            }
            case 3:{
                if(istree==0||root==NULL){
                    printf("哈夫曼树未构建\n");
                    break;
                }
                printtree(root);
                break;
            }
            case 4:{
                if(istree==0||root==NULL){
                    printf("哈夫曼树未构建\n");
                    break;
                }

                if(isfile){
                    printf("提示：将重新生成本次的编码文件与编码表\n");
                }
                memset(temporarary,0,sizeof(temporarary));
                memset(codetable,0,sizeof(codetable));

                createtable(root,temporarary,0,codetable);

                char outfile[NAME_MAX_LEN];char outfile2[NAME_MAX_LEN];
                printf("请输入生成的文件名（英文）");
                if(!readLine(outfile,sizeof(outfile))||outfile[0]=='\0'){
                    printf("文件名无效\n");
                    break;
                }
                printf("请输入生成的编码表文件名（英文）");
                if(!readLine(outfile2,sizeof(outfile2))||outfile2[0]=='\0'){
                    printf("文件名无效\n");
                    break;
                }

                if(!putoutfile(filename,outfile,totalchars,codetable)){
                    printf("编码文件生成失败\n");
                    break;
                }
                if(!savecodetable(outfile2,codetable)){
                    printf("编码表文件生成失败\n");
                    break;
                }

                isfile=1;
                strcpy(codefile,outfile);
                strcpy(tablefile,outfile2);
                printcodetable(codetable);
                break;
            }
            case 5:{
                char table[NAME_MAX_LEN];
                char file[NAME_MAX_LEN];
                char outfile[NAME_MAX_LEN];
                printf("请输入编码表文件名:");
                if(!readLine(table,sizeof(table))||table[0]=='\0'){printf("文件名无效\n");break;}

                printf("请输入编码文件名:");
                if(!readLine(file,sizeof(file))||file[0]=='\0'){printf("文件名无效\n");break;}
                printf("请输入输出文件名:");
                if(!readLine(outfile,sizeof(outfile))||outfile[0]=='\0'){printf("文件名无效\n");break;}
                decode(table,file,outfile);
                break;
            }
            case 6:{
                if(isfile==0||codefile[0]=='\0'||totalchars<=0){
                    printf("请先完成步骤 1～4 生成编码文件\n");
                    break;
                }
                printcompressionratio(codefile,tablefile,totalchars,freq,codetable);
                break;
            }
            case 7:{

                freetree(root);
                root=NULL;
                return 0;
            }

            default:{
                printf("无效的选择，请选其他\n");
                break;
            }

        }

    }
    freetree(root);
    return 0;

}
