const char AG_HTTP_HEAD_START[]        = "<!DOCTYPE html>"
"<html lang='en'><head>"
"<meta name='format-detection' content='telephone=no'>"
"<meta charset='UTF-8'>"
"<meta  name='viewport' content='width=device-width,initial-scale=1,user-scalable=no'/>";

const char AG_HTTP_HEAD_END[]          = "</head><body><div class='wrap'>"; // {c} = _bodyclass
const char AG_HTTP_END[]               = "</div></body></html>";
//
//// example of embedded logo, base64 encoded inline, No styling here
//const char HTTP_ROOT_MAIN[]         = "<h1>{t}</h1><h3>{v}</h3>";
//
//const char HTTP_PORTAL_OPTIONS[]    = "";
//const char HTTP_ITEM_QI[]           = "<div role='img' aria-label='{r}%' title='{r}%' class='q q-{q} {i} {h}'></div>"; // rssi icons
//const char HTTP_ITEM_QP[]           = "<div class='q {h}'>{r}%</div>"; // rssi percentage {h} = hidden showperc pref
//const char HTTP_ITEM[]              = "<div><a href='#p' onclick='c(this)' data-ssid='{V}'>{v}</a>{qi}{qp}</div>"; // {q} = HTTP_ITEM_QI, {r} = HTTP_ITEM_QP
//// const char HTTP_ITEM[]             = "<div><a href='#p' onclick='c(this)'>{v}</a> {R} {r}% {q} {e}</div>"; // test all tokens
//
//const char HTTP_FORM_START[]        = "<form method='POST' action='{v}'>";
//const char HTTP_FORM_WIFI[]         = "<label for='s'>SSID</label><input id='s' name='s' maxlength='32' autocorrect='off' autocapitalize='none' placeholder='{v}'><br/><label for='p'>Password</label><input id='p' name='p' maxlength='64' type='password' placeholder='{p}'><input type='checkbox' onclick='f()'> Show Password";
//const char HTTP_FORM_WIFI_END[]     = "";
//const char HTTP_FORM_STATIC_HEAD[]  = "<hr><br/>";
//const char HTTP_FORM_END[]          = "<br/><br/><button type='submit'>Save</button></form>";
//const char HTTP_FORM_LABEL[]        = "<label for='{i}'>{t}</label>";
//const char HTTP_FORM_PARAM_HEAD[]   = "<hr><br/>";
//const char HTTP_FORM_PARAM[]        = "<br/><input id='{i}' name='{n}' maxlength='{l}' value='{v}' {c}>\n"; // do not remove newline!
//
//const char HTTP_SCAN_LINK[]         = "<br/><form action='/wifi?refresh=1' method='POST'><button name='refresh' value='1'>Refresh</button></form>";
//const char HTTP_SAVED[]             = "<div class='msg'>Saving Credentials<br/>Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
//const char HTTP_PARAMSAVED[]        = "<div class='msg S'>Saved<br/></div>";
//const char HTTP_ERASEBTN[]          = "<br/><form action='/erase' method='get'><button class='D'>Erase WiFi Config</button></form>";
//const char HTTP_UPDATEBTN[]         = "<br/><form action='/update' method='get'><button>Update</button></form>";
//const char HTTP_BACKBTN[]           = "<hr><br/><form action='/' method='get'><button>Back</button></form>";
//
//const char HTTP_STATUS_ON[]         = "<div class='msg S'><strong>Connected</strong> to {v}<br/><em><small>with IP {i}</small></em></div>";
//const char HTTP_STATUS_OFF[]        = "<div class='msg {c}'><strong>Not Connected</strong> to {v}{r}</div>"; // {c=class} {v=ssid} {r=status_off}
//const char HTTP_STATUS_OFFPW[]      = "<br/>Authentication Failure"; // STATION_WRONG_PASSWORD,  no eps32
//const char HTTP_STATUS_OFFNOAP[]    = "<br/>AP not found";   // WL_NO_SSID_AVAIL
//const char HTTP_STATUS_OFFFAIL[]    = "<br/>Could not Connect"; // WL_CONNECT_FAILED
//const char HTTP_STATUS_NONE[]       = "<div class='msg'>No AP set</div>";
//const char HTTP_BR[]                = "<br/>";

const char AG_HTTP_STYLE[]             = "<style>"
".c,body{text-align:center;font-family:verdana}div,input,select{padding:5px;font-size:1em;margin:5px 0;box-sizing:border-box}"
"input,button,select,.msg{border-radius:.3rem;width: 100%}input[type=radio],input[type=checkbox]{width:auto}"
"button,input[type='button'],input[type='submit']{cursor:pointer;border:0;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%}"
"input[type='file']{border:1px solid #1fa3ec}"
".wrap {text-align:left;display:inline-block;min-width:260px;max-width:500px}"
// links
"a{color:#000;font-weight:700;text-decoration:none}a:hover{color:#1fa3ec;text-decoration:underline}"
// quality icons
".q{height:16px;margin:0;padding:0 5px;text-align:right;min-width:38px;float:right}.q.q-0:after{background-position-x:0}.q.q-1:after{background-position-x:-16px}.q.q-2:after{background-position-x:-32px}.q.q-3:after{background-position-x:-48px}.q.q-4:after{background-position-x:-64px}.q.l:before{background-position-x:-80px;padding-right:5px}.ql .q{float:left}.q:after,.q:before{content:'';width:16px;height:16px;display:inline-block;background-repeat:no-repeat;background-position: 16px 0;"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAGAAAAAQCAMAAADeZIrLAAAAJFBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADHJj5lAAAAC3RSTlMAIjN3iJmqu8zd7vF8pzcAAABsSURBVHja7Y1BCsAwCASNSVo3/v+/BUEiXnIoXkoX5jAQMxTHzK9cVSnvDxwD8bFx8PhZ9q8FmghXBhqA1faxk92PsxvRc2CCCFdhQCbRkLoAQ3q/wWUBqG35ZxtVzW4Ed6LngPyBU2CobdIDQ5oPWI5nCUwAAAAASUVORK5CYII=');}"
// icons @2x media query (32px rescaled)
"@media (-webkit-min-device-pixel-ratio: 2),(min-resolution: 192dpi){.q:before,.q:after {"
"background-image:url('data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAALwAAAAgCAMAAACfM+KhAAAALVBMVEX///8AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAADAOrOgAAAADnRSTlMAESIzRGZ3iJmqu8zd7gKjCLQAAACmSURBVHgB7dDBCoMwEEXRmKlVY3L//3NLhyzqIqSUggy8uxnhCR5Mo8xLt+14aZ7wwgsvvPA/ofv9+44334UXXngvb6XsFhO/VoC2RsSv9J7x8BnYLW+AjT56ud/uePMdb7IP8Bsc/e7h8Cfk912ghsNXWPpDC4hvN+D1560A1QPORyh84VKLjjdvfPFm++i9EWq0348XXnjhhT+4dIbCW+WjZim9AKk4UZMnnCEuAAAAAElFTkSuQmCC');"
"background-size: 95px 16px;}}"
// msg callouts
".msg{padding:20px;margin:20px 0;border:1px solid #eee;border-left-width:5px;border-left-color:#777}.msg h4{margin-top:0;margin-bottom:5px}.msg.P{border-left-color:#1fa3ec}.msg.P h4{color:#1fa3ec}.msg.D{border-left-color:#dc3630}.msg.D h4{color:#dc3630}.msg.S{border-left-color: #5cb85c}.msg.S h4{color: #5cb85c}"
// lists
"dt{font-weight:bold}dd{margin:0;padding:0 0 0.5em 0;min-height:12px}"
"td{vertical-align: top;}"
".h{display:none}"
"button{transition: 0s opacity;transition-delay: 3s;transition-duration: 0s;cursor: pointer}"
"button.D{background-color:#dc3630}"
"button:active{opacity:50% !important;cursor:wait;transition-delay: 0s}"
// invert
"body.invert,body.invert a,body.invert h1 {background-color:#060606;color:#fff;}"
"body.invert .msg{color:#fff;background-color:#282828;border-top:1px solid #555;border-right:1px solid #555;border-bottom:1px solid #555;}"
"body.invert .q[role=img]{-webkit-filter:invert(1);filter:invert(1);}"
":disabled {opacity: 0.5;}"
"</style>";
