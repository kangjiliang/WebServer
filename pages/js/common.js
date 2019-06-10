
function getUniqueID(){
    return Date.now().toString() + Math.random().toString().substr(3);
}

//设置cookie
function setCookie(cname, cvalue, exdays) {
    var d = new Date();
    d.setTime(d.getTime() + (exdays*24*60*60*1000));
    var expires = "expires="+d.toUTCString();
    document.cookie = cname + "=" + cvalue + "; " + expires;
}

//获取cookie
function getCookie(cname)
{
    var name = cname + "=";
    var ca   = document.cookie.split(';');
    for(var i=0; i<ca.length; i++)
    {
        var c = ca[i];
        while (c.charAt(0)==' ') c = c.substring(1);
        if (c.indexOf(name) != -1) return c.substring(name.length, c.length);
    }
    return "";
}

function getCheckedRadioValue(name){
    var radios = document.getElementsByName(name);
    for(var i = 0; i < radios.length; i++){
        if(radios[i].checked == true){
            return radios[i].value
        }
    }
    return "";
}

function ajax_post_json2(cgiurl, jsonobj, successfunc){
    var xmlhttp;
    if (window.XMLHttpRequest){// code for IE7+, Firefox, Chrome, Opera, Safari
        xmlhttp=new XMLHttpRequest();
    }
    else{// code for IE6, IE5
        xmlhttp=new ActiveXObject("Microsoft.XMLHTTP");
    }
    xmlhttp.onreadystatechange=function(){
        if (xmlhttp.readyState == 4 ){
            if(xmlhttp.status == 200){
                alert(xmlhttp.responseText);
                //alert(successfunc)
                //successfunc(xmlhttp.responseText, xmlhttp.status);
            }
            else {
                alert(xmlhttp.responseText);
            }
        };
    }
    xmlhttp.open("POST", cgiurl, true);
    xmlhttp.setRequestHeader("Content-Type","application/json; charset=utf-8;");
    xmlhttp.send(JSON.stringify(jsonobj));
}

function ajax_post_json(cgiurl, jsonobj, successfunc, errorfunc)
{
    //jsonobj.r = Math.random();
    //jsonobj.t = new Date();
    //url: cgiurl+"?"+Math.random(),
    $.ajax({
        url: cgiurl+"?"+Math.random(),
        type: "POST",
        cache: false,
        dataType: "text",
        data: JSON.stringify(jsonobj),
        processData: false,
        contentType: "application/json; charset=utf-8;",
        success: function(data, textStatus){
                    if(successfunc)
                        successfunc(data, textStatus);
                    else{
                        alert(data);
                    }
                 },
        error: function(XMLHttpRequest, textStatus, errorThrown){
                    if(errorfunc){
                        errorfunc(XMLHttpRequest, textStatus, errorThrown);
                    }
                    else{
                        alert(XMLHttpRequest.responseText);
                    }
                }
    });
}

function ajax_post_multipart(cgiurl, formid, successfunc)
{
    var formdata = new FormData($("#" + formid)[0]);

    $.ajax({
        url: cgiurl,
        type: "POST",
        cache: false,
        data: formdata,
        processData: false,
        contentType: false,
        success: function(data, textStatus){successfunc(data, textStatus);},
        error: function(XMLHttpRequest, textStatus, errorThrown){alert(XMLHttpRequest.responseText);}
    });
}

function websocket_test()
{
    if ("WebSocket" in window)
    {
        // 打开一个 web socket
        var ws = new WebSocket("ws://192.168.0.118:8888");
        ws.onopen = function()
        {
            // Web Socket 已连接上，使用 send() 方法发送数据
            var message = "1234567890";
            ws.send(message);
            alert("发送 " + message);
        };
        ws.onmessage = function (evt)
        {
            alert("接收 " + evt.data);
        };
        ws.onclose = function()
        {
            // 关闭 websocket
            alert("连接已关闭...");
        };
    }
    else
    {
        // 浏览器不支持 WebSocket
        alert("浏览器不支持 WebSocket!");
    }
}


// 表格边框
//1、显示表格的4个边框：<table border frame=box>
//2、只显示上边框:      <table border frame=above>
//3、只显示下边框:      <table border frame=below>
//4、只显示上下边框:    <table border frame=hsides>
//5、只显示左右边框:    <table border frame=vsides>
//6、只显示左边框：     <table border frame=lhs>
//7、只显示右边框：     <table border frame=rhs>
//8、不显示任何边框：   <table border frame=void>

//1、只显示列与列之间的分隔线：<table rules=cols>
//2、只显示行与行之间的分隔线：<table rules=rows>
//3、不显示任何分隔线：        <table rules=none>     (只显示四个边框)
//4、显示所有分隔线：          <table rules=all>
