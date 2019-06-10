
function webztree_onAsyncError(event, treeId, treeNode, XMLHttpRequest, textStatus, errorThrown) {
    alert(XMLHttpRequest.responseText);
};

function webztree_OnRightClick(event, treeId, treeNode){
    if(treeNode){
        webztreeobj.selectNode(treeNode);
    }else{
        webztreeobj.cancelSelectedNode();
    }
    webztree_rmenu_show(event.clientX, event.clientY);            
}

function webztree_rmenu_show(x, y){
    webztreermenu.show();
    webztreermenu.find('li').each(function(){
        $(this).show();
    });
    y += document.body.scrollTop;
    x += document.body.scrollLeft;
    webztreermenu.css({"top":y+"px", "left":x+"px", "visibility":"visible"});
}

function webztree_rmenu_hide() {
    if(webztreermenu) webztreermenu.css({"visibility": "hidden"});
}

function webztree_addnode() {
    webztree_rmenu_hide();
    var nodelist = webztreeobj.getSelectedNodes();
    var nodename = prompt("请输入新增节点的名字：");
    if(null == nodename){
        return;
    }
    var pagename = prompt("请输入新增网页的名字：");
    if(null == pagename){
        return;
    }

    var newnode    = {};
    newnode.name   = nodename;
    newnode.id     = getUniqueID();
    newnode.url    = pagename;
    newnode.target = "main";
    newnode.pId    = nodelist[0] ? nodelist[0].id : "0";
    newnode.open   = "true";
    
    var jsonobj = {};
    jsonobj.cgitype = "addnode";
    jsonobj.treeId  = webztreeobj.setting.treeId;
    jsonobj.newnode = newnode;
    ajax_post_json("/cgibin/cgi_process_webztree.py", jsonobj, webztree_addnode_success);
}

function webztree_addnode_success(data, textStatus){
    var newnode = JSON.parse(data);
    if (webztreeobj.getSelectedNodes()[0]) {
        webztreeobj.addNodes(webztreeobj.getSelectedNodes()[0], newnode);
    } else {
        webztreeobj.addNodes(null, newnode);
    }    
}


function webztree_delnode() {
    webztree_rmenu_hide();
    var nodes = webztreeobj.getSelectedNodes();
    if (nodes && nodes.length>0) {
        if (nodes[0].children && nodes[0].children.length > 0) {
            if (confirm("要删除的是父节点，将连同子节点一起删掉\n请确认！")==false){
                return;
            }
        }else{
            if (confirm("确认要删除此节点吗？")==false){
                return;
            }
        }
        var delpass = prompt("请输入删除密码：");
        if(null == delpass){
            return;
        }        

        var jsonobj = {};
        jsonobj.cgitype = "delnode";
        jsonobj.treeId  = webztreeobj.setting.treeId;
        jsonobj.delnode = nodes[0];
        jsonobj.delpass = delpass;
        ajax_post_json("/cgibin/cgi_process_webztree.py", jsonobj, function(){webztree_delnode_success(nodes[0]);});
    }
}

function webztree_delnode_success(node){
    //var delnode = JSON.parse(data);
    webztreeobj.removeNode(node);
}

function webztree_setting(treeId){
    var setting = {
        data: {
            simpleData: {
                enable: true
            }
        },
        async: {
            enable: true,
            url:"../cgibin/cgi_process_webztree.py",
            otherParam:{"cgitype":"loadztree", "treeId":treeId },
        },
        callback:{
            onAsyncError: webztree_onAsyncError,
            onRightClick: webztree_OnRightClick,
        },
        view:{
            dblClickExpand: true
        },
    };
    return setting;
}

function webztree_rmenu_addbutton(rmenuul, func, text){
    var rmenuli = $('<li></li>');
    rmenuli.click(func);
    rmenuli.text(text);
    rmenuli.appendTo(rmenuul);
}

function webztree_rmenu_item(name, func){
    var rmenuli = $('<li></li>');
    rmenuli.click(func);
    rmenuli.text(name);
    return rmenuli;
}

function webztree_create_rmenu(rmenuid, items){
    var rmenudiv = $('<div></div>');
    rmenudiv.attr('id', rmenuid); 
    rmenudiv.appendTo('body');

    var rmenuul = $('<ul></ul>');
    rmenuul.appendTo(rmenudiv);

    for (var i = 0; i < items.length; i++)
    {
        item = webztree_rmenu_item(items[i].name, items[i].func);
        item.appendTo(rmenuul);
    }
    webztree_rmenu_hide();
    return rmenudiv;
}


function webztree_getpage_success(){}

function webztree_editpage() {
    webztree_rmenu_hide();
    var nodes = webztreeobj.getSelectedNodes();
    if(nodes && nodes.length > 0) {
        var tmpform     = document.createElement('form');
        tmpform.style   = "display:none;";
        tmpform.method  = 'get';
        tmpform.action  = '/cgibin/cgi_process_webztree.py?abc=456';
        tmpform.target  = 'main';
        var tmpinput1   = document.createElement('input');
        tmpinput1.type  = 'hidden';
        tmpinput1.name  = 'cgitype';
        tmpinput1.value = 'editpage';
        tmpform.appendChild(tmpinput1);
        var tmpinput2   = document.createElement('input');
        tmpinput2.type  = 'hidden';
        tmpinput2.name  = 'pageurl';
        tmpinput2.value = nodes[0].url;
        tmpform.appendChild(tmpinput2);

        document.body.appendChild(tmpform);
        tmpform.submit();
        document.body.removeChild(tmpform);
    }
}




var webztreeobj;
var webztreermenu;
var webztreermenuitems = [  
    {name:"新建网页", func:webztree_addnode},
    {name:"删除网页", func:webztree_delnode},    
    {name:"编辑网页", func:webztree_editpage},
];


function webztree_create(treeId ){
    $.fn.zTree.init($("#"+treeId ), webztree_setting(treeId ));
    webztreeobj = $.fn.zTree.getZTreeObj(treeId );
    webztreermenu = webztree_create_rmenu("ZTREE_RMENU", webztreermenuitems);    
}
