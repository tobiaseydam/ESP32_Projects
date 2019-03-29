## Beispiel: HttpRequestParser

Für dieses Beispiel sollte die maximale Request Header-Länge erhöht werden:

```
make menuconfig
Component config -> HTTP Server -> Max HTTP Request Header Length: 1024
Component config -> HTTP Server -> Max HTTP URI Length: 1024 (?)
```

Zum korrekten parsen:

```html
<form method="POST" action="http://ESP32_IP_ADDR/test" enctype="multipart/form-data">
    ...
</form>
```

