#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/stat.h>
#include"cgic.h"
#include <stdlib.h>


#define BufferLen 1024

int cgiMain(void){
	cgiFilePtr file;
	int targetFile;
	mode_t mode;
	char name[128];
	char nameForCheck[64];
	char fileNameOnServer[64];
	char contentType[1024];
	char buffer[BufferLen];
	char *tmpStr=NULL;
	int size;
	int got,t;

	//取得html页面中file元素的值--文件在客户机上的路径名
	if (cgiFormFileName("file", name, sizeof(name)) !=cgiFormSuccess) {
	}
	cgiFormFileSize("file", &size);
	//取得文件类型
	cgiFormFileContentType("file", contentType, sizeof(contentType));
	//目前文件存在于系统临时文件夹中，通常为/tmp，通过该命令打开临时文件
	if (cgiFormFileOpen("file", &file) != cgiFormSuccess) {
		goto FAIL;
	}
	t=-1;
	//从路径名解析出用户文件名
	while(1){
		tmpStr=strstr(name+t+1,"\\");
		if(NULL==tmpStr)
			tmpStr=strstr(name+t+1,"/");//if "\\" is not path separator, try "/"
		if(NULL!=tmpStr)
			t=(int)(tmpStr-name);
		else
			break;
	}
	strcpy(fileNameOnServer,name+t+1);
	mode=S_IRWXU|S_IRGRP|S_IROTH;
	//在当前目录下建立新的文件，第一个参数实际上是路径名，此处的含义是在cgi程序所在的目录（当前目录））建立新文件
	targetFile=open(fileNameOnServer,O_RDWR|O_CREAT|O_TRUNC|O_APPEND,mode);
	if(targetFile<0){
		goto FAIL;
	}
	//从系统临时文件中读出文件内容，并放到刚创建的目标文件中
	while (cgiFormFileRead(file, buffer, BufferLen, &got) ==cgiFormSuccess){
		if(got>0)
			write(targetFile,buffer,got);
	}
	cgiFormFileClose(file);
	close(targetFile);
	goto END;

	FAIL:
		cgiHeaderLocation("../upgrade_error.html");
		system("rm /tmp/*");
		return 1;
	END:
		//升级动作
		if(cgiFormString("upfile1",nameForCheck,64)==cgiFormSuccess){
			system("mv /app/www/cgi-bin/rtu /app/");
			system("chmod 777 /app/rtu");
		}else if(cgiFormString("upfile2",nameForCheck,64)==cgiFormSuccess){
			system("");//预留，以后做好安装包再说
		}

		//以下为网页部分
		printf("Content-type: text/html\r\n\r\n");
		printf("<html>");
		printf("<head>");
		printf("    <meta charset=\"utf-8\" content=\"text/html\">");
		printf("    <title>");
		printf("        ctt-2000e动环设备");
		printf("    </title>");
		printf("</head>");
		printf("<body style=\"background:#1782dd\">");
		printf("<div id=\"all\" style=\"width:500px;margin:250px auto;color:#fff;font-size:30px;text-align:center;line-height:60px;\">");
		printf("    升级成功<br>系统将在 <i id=\"time\" style=\"font-size:45px;\">5</i> 秒后重启");
		printf("</div>");
		printf("<script type=\"text/javascript\">");
		printf("    var id_of_time=setInterval(setTime,1000);");
		printf("    setTimeout(close,5000);");
		printf("    function setTime(){");
		printf("        var lastTime=document.getElementById(\"time\");");
		printf("        var number=lastTime.innerHTML;");
		printf("        number=number-1;");
		printf("        lastTime.innerHTML=number;");
		printf("    }");
		printf("    function close(){");
		printf("	clearInterval(id_of_time);");
		printf("	var all=document.getElementById(\"all\");");
		printf("	all.innerHTML=\"系统重启中，请稍后点此重试连接\";");
		printf("	all.onclick=function(){");
		printf("	    window.open(\"http://\"+location.hostname+\":\"+location.port,\"_self\",\"\");");
		printf("        };");
		printf("    }");
		printf("</script>");
		printf("</body>");
		printf("</html>");

		system("rm /tmp/*");
		system("/sbin/reboot");
		return 0;
}






