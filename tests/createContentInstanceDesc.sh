curl --request POST --url 'http://127.0.0.1:8080/~/in-cse/in-name/MY_SENSOR/DESCRIPTOR' --header 'content-type: application/xml;ty=4' --header 'x-m2m-origin: admin:admin' --data '<m2m:cin xmlns:m2m="http://www.onem2m.org/xml/protocols"><cnf>application/xml</cnf><con>&lt;obj&gt;&lt;str name=&quot;type&quot; val=&quot;Temperature_Sensor&quot;/&gt;&lt;str name=&quot;location&quot; val=&quot;Home&quot;/&gt;&lt;str name=&quot;appId&quot; val=&quot;MY_SENSOR&quot;/&gt;&lt;op name=&quot;getValue&quot; href=&quot;/in-cse/in-name/MY_SENSOR/DATA/la&quot;in=&quot;obix:Nil&quot; out=&quot;obix:Nil&quot; is=&quot;retrieve&quot;/&gt;&lt;/obj&gt;</con></m2m:cin>'