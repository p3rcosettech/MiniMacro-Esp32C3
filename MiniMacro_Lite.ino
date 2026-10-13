/*
  MiniMacro Lite — ESP32-C3 SuperMini + KY-040
  ------------------------------------------------------------------
  Encoder pins: CLK=GPIO8, DT=GPIO10, SW=GPIO9
  Wi-Fi AP:     MiniMacro Lite / macroC3lite
  Web UI:       http://192.168.4.1
  BLE HID name: MiniMacro Lite

  Dependencies:
    - ESP32 Arduino core 3.x
    - NimBLE-Arduino >= 2.3.8
    - HijelHID_BLEKeyboard

  Note: GPIO8 and GPIO9 are ESP32-C3 strapping pins. Keep the encoder
  button released during reset/power-up. See README.md.
*/

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <HijelHID_BLEKeyboard.h>

// ------------------------------- Hardware -------------------------------
static constexpr uint8_t PIN_ENC_CLK = 8;
static constexpr uint8_t PIN_ENC_DT  = 10;
static constexpr uint8_t PIN_ENC_SW  = 9;

static const char *AP_SSID = "MiniMacro Lite";
static const char *AP_PASS = "macroC3lite";
static constexpr uint16_t WEB_PORT = 80;

WebServer server(WEB_PORT);
Preferences prefs;
HijelHID_BLEKeyboard keyboard("MiniMacro Lite", "MiniMacro", 100);

// ---------------------------- Macro defaults ----------------------------
struct EventDef {
  const char *id;
  const char *title;
  const char *description;
  const char *defaultMode;
  const char *defaultValue;
};

static const EventDef EVENTS[] = {
  {"left",     "Поворот влево",             "Обычное вращение против часовой стрелки", "media",    "volDown"},
  {"right",    "Поворот вправо",            "Обычное вращение по часовой стрелке",     "media",    "volUp"},
  {"click",    "Короткое нажатие",          "Один щелчок энкодера",                     "media",    "playPause"},
  {"double",   "Двойное нажатие",           "Два быстрых щелчка",                       "media",    "next"},
  {"hold",     "Долгое нажатие",            "Удержание около 650 мс",                  "media",    "mute"},
  {"leftHold", "Вращение влево с нажатием", "Поворот, пока кнопка удерживается",       "shortcut", "CTRL+LEFT"},
  {"rightHold","Вращение вправо с нажатием","Поворот, пока кнопка удерживается",       "shortcut", "CTRL+RIGHT"}
};
static constexpr size_t EVENT_COUNT = sizeof(EVENTS) / sizeof(EVENTS[0]);

// ------------------------------- Web UI ---------------------------------
static const char INDEX_HTML[] PROGMEM = R"MMHTML(
<!doctype html>
<html lang="ru">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#10111a"><title>MiniMacro Lite</title>
<style>
:root{color-scheme:dark;--bg:#0d0e15;--panel:rgba(27,29,43,.82);--stroke:rgba(255,255,255,.09);--text:#f4f5ff;--muted:#a3a8c1;--a:#a78bfa;--b:#6ee7d2;--danger:#ff8d9b}
*{box-sizing:border-box}html{min-height:100%;background:var(--bg)}body{margin:0;color:var(--text);font:15px/1.45 ui-sans-serif,system-ui,-apple-system,"Segoe UI",Roboto,Arial,sans-serif;background:radial-gradient(ellipse at 6% 0%,rgba(126,87,194,.22),transparent 36rem),radial-gradient(ellipse at 95% 22%,rgba(32,201,181,.12),transparent 32rem),var(--bg);min-height:100vh}
button,input,select{font:inherit}button,select,input{border-radius:13px}button{cursor:pointer;border:1px solid var(--stroke);color:var(--text);background:#26283a;padding:10px 14px;transition:transform .12s,background .12s}button:active{transform:scale(.98)}button:focus-visible,input:focus-visible,select:focus-visible{outline:2px solid var(--b);outline-offset:2px}.primary{background:linear-gradient(115deg,#8b5cf6,#6475ed);border:0;font-weight:750;box-shadow:0 8px 28px #775be32b}.secondary{background:#26283a}.small{padding:8px 12px;font-size:13px}.wrap{width:min(1040px,100%);margin:auto;padding:22px 16px 46px}.top{display:flex;align-items:center;justify-content:space-between;gap:16px;margin:5px 0 20px}.brand{display:flex;align-items:center;gap:13px}.logo{width:48px;height:48px;display:grid;place-items:center;border-radius:17px;background:linear-gradient(145deg,#a78bfa,#4e68d7);font-size:25px;box-shadow:0 10px 35px #7c5ce63b}.brand h1{font-size:clamp(22px,4vw,30px);letter-spacing:-.7px;margin:0}.sub{margin:2px 0 0;color:var(--muted);font-size:13px}.badge{display:inline-flex;align-items:center;gap:8px;padding:9px 12px;border:1px solid var(--stroke);background:#202232aa;border-radius:999px;color:#d7d9ed;white-space:nowrap;font-size:12px}.dot{width:7px;height:7px;border-radius:50%;background:#9ca3af;box-shadow:0 0 12px currentColor}.dot.good{background:#6ee7b7;color:#6ee7b7}.hero{padding:23px;border:1px solid var(--stroke);border-radius:24px;background:linear-gradient(130deg,rgba(38,39,60,.92),rgba(25,29,42,.77));box-shadow:0 16px 50px #0002;margin-bottom:18px;overflow:hidden;position:relative}.hero:after{content:"";position:absolute;width:210px;height:210px;border-radius:50%;right:-75px;top:-105px;background:#8b5cf624;filter:blur(3px);pointer-events:none}.hero h2{font-size:21px;letter-spacing:-.4px;margin:0 0 7px}.hero p{margin:0;color:var(--muted);max-width:650px}.stats{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:21px;position:relative;z-index:1}.stat{padding:12px 14px;border-radius:15px;background:#1214207a;border:1px solid #ffffff0b;min-width:0}.stat .label{color:var(--muted);font-size:12px;margin-bottom:5px}.stat strong{font-size:14px;overflow-wrap:anywhere}.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:14px}.card{min-width:0;border:1px solid var(--stroke);background:var(--panel);backdrop-filter:blur(14px);border-radius:20px;padding:17px;box-shadow:0 10px 35px #00012}.section-title{display:flex;align-items:flex-start;justify-content:space-between;gap:12px;margin:2px 0 15px}.section-title h2{margin:0;font-size:17px;letter-spacing:-.25px}.section-title p{margin:4px 0 0;color:var(--muted);font-size:12px}.actions{display:flex;flex-direction:column;gap:10px}.action{background:#131520a6;border:1px solid #ffffff0b;border-radius:15px;padding:13px}.action-head{display:flex;align-items:flex-start;justify-content:space-between;gap:8px;margin-bottom:11px}.action-title{font-size:14px;font-weight:750}.action-desc{font-size:11px;color:var(--muted);margin-top:3px}.pill{font-size:10px;border:1px solid #a78bfa42;background:#a78bfa13;color:#d3c6ff;border-radius:999px;padding:4px 7px;white-space:nowrap}.fields{display:grid;grid-template-columns: minmax(115px,.8fr) minmax(0,1.2fr);gap:8px}.fields select,.fields input{width:100%;min-width:0;background:#222437;color:var(--text);border:1px solid #ffffff14;padding:10px 11px;outline:none}.fields input::placeholder{color:#747b98}.value-wrap[hidden],.media-wrap[hidden]{display:none!important}.foot{display:flex;align-items:center;justify-content:space-between;gap:8px;margin-top:10px}.help{font-size:11px;color:var(--muted)}.bottom-actions{display:flex;gap:9px;flex-wrap:wrap;margin-top:15px}.wide{grid-column:1/-1}.network-row{display:flex;justify-content:space-between;align-items:center;gap:12px;padding:9px 0;border-bottom:1px solid #ffffff0c}.network-row:last-child{border-bottom:0}.network-row span{color:var(--muted);font-size:13px}.network-row strong{font-size:13px;text-align:right;overflow-wrap:anywhere}.notice{font-size:12px;color:var(--muted);background:#1214209c;padding:12px 13px;border-radius:13px;border:1px solid #ffffff0a;margin-top:13px}.notice b{color:#e5e7ff}.footer{text-align:center;color:#737990;font-size:11px;margin:18px 0 0}.toast{position:fixed;left:50%;bottom:22px;transform:translate(-50%,20px);opacity:0;pointer-events:none;background:#25283c;border:1px solid #ffffff1a;color:var(--text);padding:12px 16px;border-radius:14px;box-shadow:0 12px 35px #0007;transition:.2s;max-width:calc(100vw - 30px);z-index:20}.toast.show{opacity:1;transform:translate(-50%,0)}.toast.error{border-color:#ff8d9b66}
@media(max-width:720px){.wrap{padding:16px 12px 34px}.top{align-items:flex-start;flex-direction:column}.badge{align-self:flex-start}.grid{grid-template-columns:1fr}.hero{padding:19px}.stats{gap:7px}.stat{padding:11px 9px}.stat strong{font-size:12px}.card{padding:14px}.fields{grid-template-columns:minmax(105px,.82fr) minmax(0,1.18fr)}}
</style>
</head>
<body>
<div class="wrap">
<header class="top"><div class="brand"><div class="logo">◉</div><div><h1>MiniMacro <span style="color:var(--a)">Lite</span></h1><p class="sub">Твой энкодер. Твои команды. Без лишних приложений.</p></div></div><div id="bleBadge" class="badge"><i id="bleDot" class="dot"></i><span id="bleLabel">BLE: проверка…</span></div></header>
<section class="hero"><h2>Control center</h2><p>Настрой вращение и нажатия KY-040. Команды отправляются как Bluetooth-клавиатура, а все настройки сохраняются во флеш-памяти платы.</p><div class="stats"><div class="stat"><div class="label">Bluetooth HID</div><strong id="bleStat">Проверяем связь…</strong></div><div class="stat"><div class="label">Wi-Fi панель</div><strong>Активна</strong></div><div class="stat"><div class="label">Подключено к AP</div><strong id="clientsStat">—</strong></div></div></section>
<div class="grid">
<section class="card"><div class="section-title"><div><h2>Назначения энкодера</h2><p>Выбери событие и укажи действие</p></div><span class="pill">7 событий</span></div>
<div class="actions" id="actions">
<div class="action" data-id="left"><div class="action-head"><div><div class="action-title">Поворот влево</div><div class="action-desc">Обычное вращение против часовой стрелки</div></div><span class="pill">ROTATE</span></div><div class="fields"><select class="mode" aria-label="Режим действия"><option value="none">Ничего</option><option value="media">Медиа</option><option value="shortcut">Сочетание клавиш</option><option value="text">Ввести текст</option></select><div class="media-wrap"><select class="media" aria-label="Медиа-команда"><option value="volDown">Громкость −</option><option value="volUp">Громкость +</option><option value="mute">Без звука</option><option value="playPause">Play / Pause</option><option value="next">Следующий трек</option><option value="prev">Предыдущий трек</option></select></div><div class="value-wrap" hidden><input class="value" maxlength="180" autocomplete="off" placeholder="CTRL+SHIFT+S" aria-label="Сочетание клавиш или текст"></div></div><div class="foot"><span class="help">Можно проверить перед использованием</span><button class="small secondary test" type="button">Тест</button></div></div>
<div class="action" data-id="right"><div class="action-head"><div><div class="action-title">Поворот вправо</div><div class="action-desc">Обычное вращение по часовой стрелке</div></div><span class="pill">ROTATE</span></div><div class="fields"><select class="mode" aria-label="Режим действия"><option value="none">Ничего</option><option value="media">Медиа</option><option value="shortcut">Сочетание клавиш</option><option value="text">Ввести текст</option></select><div class="media-wrap"><select class="media" aria-label="Медиа-команда"><option value="volDown">Громкость −</option><option value="volUp">Громкость +</option><option value="mute">Без звука</option><option value="playPause">Play / Pause</option><option value="next">Следующий трек</option><option value="prev">Предыдущий трек</option></select></div><div class="value-wrap" hidden><input class="value" maxlength="180" autocomplete="off" placeholder="CTRL+SHIFT+S" aria-label="Сочетание клавиш или текст"></div></div><div class="foot"><span class="help">Можно проверить перед использованием</span><button class="small secondary test" type="button">Тест</button></div></div>
<div class="action" data-id="click"><div class="action-head"><div><div class="action-title">Короткое нажатие</div><div class="action-desc">Один щелчок энкодера</div></div><span class="pill">CLICK</span></div><div class="fields"><select class="mode" aria-label="Режим действия"><option value="none">Ничего</option><option value="media">Медиа</option><option value="shortcut">Сочетание клавиш</option><option value="text">Ввести текст</option></select><div class="media-wrap"><select class="media" aria-label="Медиа-команда"><option value="volDown">Громкость −</option><option value="volUp">Громкость +</option><option value="mute">Без звука</option><option value="playPause">Play / Pause</option><option value="next">Следующий трек</option><option value="prev">Предыдущий трек</option></select></div><div class="value-wrap" hidden><input class="value" maxlength="180" autocomplete="off" placeholder="CTRL+SHIFT+S" aria-label="Сочетание клавиш или текст"></div></div><div class="foot"><span class="help">Можно проверить перед использованием</span><button class="small secondary test" type="button">Тест</button></div></div>
<div class="action" data-id="double"><div class="action-head"><div><div class="action-title">Двойное нажатие</div><div class="action-desc">Два быстрых щелчка подряд</div></div><span class="pill">DOUBLE</span></div><div class="fields"><select class="mode" aria-label="Режим действия"><option value="none">Ничего</option><option value="media">Медиа</option><option value="shortcut">Сочетание клавиш</option><option value="text">Ввести текст</option></select><div class="media-wrap"><select class="media" aria-label="Медиа-команда"><option value="volDown">Громкость −</option><option value="volUp">Громкость +</option><option value="mute">Без звука</option><option value="playPause">Play / Pause</option><option value="next">Следующий трек</option><option value="prev">Предыдущий трек</option></select></div><div class="value-wrap" hidden><input class="value" maxlength="180" autocomplete="off" placeholder="CTRL+SHIFT+S" aria-label="Сочетание клавиш или текст"></div></div><div class="foot"><span class="help">Можно проверить перед использованием</span><button class="small secondary test" type="button">Тест</button></div></div>
<div class="action" data-id="hold"><div class="action-head"><div><div class="action-title">Долгое нажатие</div><div class="action-desc">Удержание кнопки примерно 650 мс</div></div><span class="pill">HOLD</span></div><div class="fields"><select class="mode" aria-label="Режим действия"><option value="none">Ничего</option><option value="media">Медиа</option><option value="shortcut">Сочетание клавиш</option><option value="text">Ввести текст</option></select><div class="media-wrap"><select class="media" aria-label="Медиа-команда"><option value="volDown">Громкость −</option><option value="volUp">Громкость +</option><option value="mute">Без звука</option><option value="playPause">Play / Pause</option><option value="next">Следующий трек</option><option value="prev">Предыдущий трек</option></select></div><div class="value-wrap" hidden><input class="value" maxlength="180" autocomplete="off" placeholder="CTRL+SHIFT+S" aria-label="Сочетание клавиш или текст"></div></div><div class="foot"><span class="help">Можно проверить перед использованием</span><button class="small secondary test" type="button">Тест</button></div></div>
<div class="action" data-id="leftHold"><div class="action-head"><div><div class="action-title">Влево + удержание</div><div class="action-desc">Поворот, пока кнопка энкодера нажата</div></div><span class="pill">SHIFT + ROTATE</span></div><div class="fields"><select class="mode" aria-label="Режим действия"><option value="none">Ничего</option><option value="media">Медиа</option><option value="shortcut">Сочетание клавиш</option><option value="text">Ввести текст</option></select><div class="media-wrap"><select class="media" aria-label="Медиа-команда"><option value="volDown">Громкость −</option><option value="volUp">Громкость +</option><option value="mute">Без звука</option><option value="playPause">Play / Pause</option><option value="next">Следующий трек</option><option value="prev">Предыдущий трек</option></select></div><div class="value-wrap" hidden><input class="value" maxlength="180" autocomplete="off" placeholder="CTRL+SHIFT+S" aria-label="Сочетание клавиш или текст"></div></div><div class="foot"><span class="help">Можно проверить перед использованием</span><button class="small secondary test" type="button">Тест</button></div></div>
<div class="action" data-id="rightHold"><div class="action-head"><div><div class="action-title">Вправо + удержание</div><div class="action-desc">Поворот, пока кнопка энкодера нажата</div></div><span class="pill">SHIFT + ROTATE</span></div><div class="fields"><select class="mode" aria-label="Режим действия"><option value="none">Ничего</option><option value="media">Медиа</option><option value="shortcut">Сочетание клавиш</option><option value="text">Ввести текст</option></select><div class="media-wrap"><select class="media" aria-label="Медиа-команда"><option value="volDown">Громкость −</option><option value="volUp">Громкость +</option><option value="mute">Без звука</option><option value="playPause">Play / Pause</option><option value="next">Следующий трек</option><option value="prev">Предыдущий трек</option></select></div><div class="value-wrap" hidden><input class="value" maxlength="180" autocomplete="off" placeholder="CTRL+SHIFT+S" aria-label="Сочетание клавиш или текст"></div></div><div class="foot"><span class="help">Можно проверить перед использованием</span><button class="small secondary test" type="button">Тест</button></div></div>
</div>
<div class="bottom-actions"><button class="primary" id="saveBtn" type="button">Сохранить настройки</button><button class="secondary" id="resetBtn" type="button">Сбросить к стандартным</button></div>
<div class="notice"><b>Сочетания клавиш:</b> например, <code>CTRL+C</code>, <code>ALT+TAB</code>, <code>CTRL+SHIFT+S</code>, <code>F5</code>, <code>LEFT</code>. Для текста выбери «Ввести текст». Ввод зависит от раскладки подключённого устройства; лучше использовать латиницу и ASCII.</div>
</section>
<aside class="card"><div class="section-title"><div><h2>Подключение и состояние</h2><p>Данные обновляются автоматически</p></div><span class="pill">LIVE</span></div>
<div class="network-row"><span>Wi-Fi сеть</span><strong>MiniMacro Lite</strong></div><div class="network-row"><span>Пароль Wi-Fi</span><strong>macroC3lite</strong></div><div class="network-row"><span>Адрес панели</span><strong>192.168.4.1</strong></div><div class="network-row"><span>Bluetooth</span><strong>MiniMacro Lite</strong></div><div class="network-row"><span>Клиенты точки доступа</span><strong id="clientsDetail">—</strong></div><div class="network-row"><span>Свободная память</span><strong id="heapStat">—</strong></div>
<div class="notice"><b>Как подключиться</b><br>1. Подключись к Wi-Fi <b>MiniMacro Lite</b>.<br>2. Открой <b>http://192.168.4.1</b> в браузере.<br>3. Для отправки клавиш отдельно подключи <b>MiniMacro Lite</b> в настройках Bluetooth на компьютере или телефоне.</div>
<div class="notice"><b>Проверка макросов</b><br>Сначала подключи Bluetooth-клавиатуру, затем открой на устройстве текстовое поле или нужное приложение. Кнопка «Тест» отправляет назначенную команду в активное окно.</div>
</aside>
</div>
<div class="footer">MiniMacro Lite · ESP32-C3 · Настройки сохраняются после перезапуска</div>
</div><div class="toast" id="toast" role="status" aria-live="polite"></div>
<script>
const $=(s,r=document)=>r.querySelector(s);const $$=(s,r=document)=>Array.from(r.querySelectorAll(s));let toastTimer;
function toast(msg,error=false){const el=$('#toast');el.textContent=msg;el.classList.toggle('error',error);el.classList.add('show');clearTimeout(toastTimer);toastTimer=setTimeout(()=>el.classList.remove('show'),2600)}
function setMode(card){const mode=$('.mode',card).value;const mw=$('.media-wrap',card),vw=$('.value-wrap',card),v=$('.value',card);mw.hidden=mode!=='media';vw.hidden=mode!=='shortcut'&&mode!=='text';v.placeholder=mode==='text'?'Текст для ввода…':'Например: CTRL+SHIFT+S';v.maxLength=mode==='text'?180:44;v.autocomplete='off';}
$$('.action').forEach(card=>{$('.mode',card).addEventListener('change',()=>setMode(card));$('.test',card).addEventListener('click',async()=>{try{const r=await fetch('/api/test',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams({id:card.dataset.id})});const j=await r.json();toast(j.message|| (j.ok?'Команда отправлена':'Не удалось отправить'),!j.ok)}catch(e){toast('Не удалось связаться с платой',true)}});setMode(card)});
async function loadConfig(){const r=await fetch('/api/config',{cache:'no-store'});if(!r.ok)throw new Error('config');const j=await r.json();$$('.action').forEach(card=>{const a=j.events[card.dataset.id];if(!a)return;$('.mode',card).value=a.mode;$('.media',card).value=a.mode==='media'?a.value:'volDown';$('.value',card).value=(a.mode==='shortcut'||a.mode==='text')?a.value:'';setMode(card)});}
async function refreshState(){try{const r=await fetch('/api/state',{cache:'no-store'});const s=await r.json();$('#bleLabel').textContent=s.paired?'BLE: подключено':(s.connected?'BLE: подключение…':'BLE: не подключено');$('#bleStat').textContent=s.paired?'Подключено и готово':(s.connected?'Устанавливаем связь':'Ожидает подключения');$('#bleDot').classList.toggle('good',s.paired);$('#clientsStat').textContent=s.clients;$('#clientsDetail').textContent=s.clients;$('#heapStat').textContent=Math.round(s.freeHeap/1024)+' КБ';}catch(e){$('#bleLabel').textContent='Плата недоступна';$('#bleStat').textContent='Нет связи с панелью';$('#bleDot').classList.remove('good')}}
$('#saveBtn').addEventListener('click',async()=>{const data=new URLSearchParams();$$('.action').forEach(card=>{const id=card.dataset.id,mode=$('.mode',card).value;data.set(id+'Mode',mode);data.set(id+'Value',mode==='media'?$('.media',card).value:$('.value',card).value)});const btn=$('#saveBtn');btn.disabled=true;btn.textContent='Сохраняем…';try{const r=await fetch('/api/save',{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:data});const j=await r.json();if(!r.ok||!j.ok)throw new Error(j.message||'save');toast('Настройки сохранены во flash-памяти')}catch(e){toast(e.message||'Ошибка сохранения',true)}finally{btn.disabled=false;btn.textContent='Сохранить настройки'}});
$('#resetBtn').addEventListener('click',async()=>{try{const r=await fetch('/api/reset',{method:'POST'});const j=await r.json();if(!r.ok||!j.ok)throw new Error('reset');await loadConfig();toast('Стандартные назначения восстановлены')}catch(e){toast('Не удалось выполнить сброс',true)}});
loadConfig().catch(()=>toast('Не удалось загрузить настройки',true));refreshState();setInterval(refreshState,2500);
</script>
</body></html>
)MMHTML";

// ------------------------- Settings / persistence -------------------------
static String prefKey(const char *prefix, const char *id) {
  return String(prefix) + "_" + id;
}

static const EventDef *findEvent(const String &id) {
  for (size_t i = 0; i < EVENT_COUNT; ++i) {
    if (id == EVENTS[i].id) return &EVENTS[i];
  }
  return nullptr;
}

static void setDefaultSettings() {
  for (size_t i = 0; i < EVENT_COUNT; ++i) {
    const String mk = prefKey("m", EVENTS[i].id);
    const String vk = prefKey("v", EVENTS[i].id);
    prefs.putString(mk.c_str(), EVENTS[i].defaultMode);
    prefs.putString(vk.c_str(), EVENTS[i].defaultValue);
  }
}

static void ensureSettings() {
  for (size_t i = 0; i < EVENT_COUNT; ++i) {
    const String mk = prefKey("m", EVENTS[i].id);
    const String vk = prefKey("v", EVENTS[i].id);
    if (!prefs.isKey(mk.c_str())) prefs.putString(mk.c_str(), EVENTS[i].defaultMode);
    if (!prefs.isKey(vk.c_str())) prefs.putString(vk.c_str(), EVENTS[i].defaultValue);
  }
}

static String jsonEscape(const String &src) {
  String out;
  out.reserve(src.length() + 8);
  for (size_t i = 0; i < src.length(); ++i) {
    const char c = src[i];
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"':  out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default: if ((uint8_t)c >= 0x20) out += c; break;
    }
  }
  return out;
}

static String getMode(const char *id) {
  const String key = prefKey("m", id);
  return prefs.getString(key.c_str(), "none");
}

static String getValue(const char *id) {
  const String key = prefKey("v", id);
  return prefs.getString(key.c_str(), "");
}

static bool isValidMode(const String &mode) {
  return mode == "none" || mode == "media" || mode == "shortcut" || mode == "text";
}

static String normalizeMedia(String value) {
  value.trim();
  if (value == "volUp" || value == "volDown" || value == "mute" ||
      value == "playPause" || value == "next" || value == "prev") return value;
  return "volUp";
}

// ------------------------------ BLE actions ------------------------------
// HID Usage IDs from the standard keyboard page.
static uint8_t keyCodeForToken(String token) {
  token.trim();
  token.toUpperCase();
  if (token.length() == 1) {
    const char c = token[0];
    if (c >= 'A' && c <= 'Z') return (uint8_t)(0x04 + (c - 'A'));
    if (c >= '1' && c <= '9') return (uint8_t)(0x1E + (c - '1'));
    if (c == '0') return 0x27;
    switch (c) {
      case '-': return 0x2D;
      case '=': return 0x2E;
      case '[': return 0x2F;
      case ']': return 0x30;
      case '\\': return 0x31;
      case ';': return 0x33;
      case '\'': return 0x34;
      case '`': return 0x35;
      case ',': return 0x36;
      case '.': return 0x37;
      case '/': return 0x38;
      default: return 0;
    }
  }
  if (token == "ENTER" || token == "RETURN") return 0x28;
  if (token == "ESC" || token == "ESCAPE") return 0x29;
  if (token == "BACKSPACE" || token == "BKSP") return 0x2A;
  if (token == "TAB") return 0x2B;
  if (token == "SPACE") return 0x2C;
  if (token == "CAPS" || token == "CAPSLOCK") return 0x39;
  if (token == "PRINTSCREEN" || token == "PRTSC") return 0x46;
  if (token == "SCROLLLOCK") return 0x47;
  if (token == "PAUSE") return 0x48;
  if (token == "INSERT" || token == "INS") return 0x49;
  if (token == "HOME") return 0x4A;
  if (token == "PAGEUP" || token == "PGUP") return 0x4B;
  if (token == "DELETE" || token == "DEL") return 0x4C;
  if (token == "END") return 0x4D;
  if (token == "PAGEDOWN" || token == "PGDN") return 0x4E;
  if (token == "RIGHT") return 0x4F;
  if (token == "LEFT") return 0x50;
  if (token == "DOWN") return 0x51;
  if (token == "UP") return 0x52;
  if (token == "MINUS") return 0x2D;
  if (token == "EQUALS") return 0x2E;
  if (token == "COMMA") return 0x36;
  if (token == "PERIOD") return 0x37;
  if (token == "SLASH") return 0x38;
  if (token.length() >= 2 && token[0] == 'F') {
    const int n = token.substring(1).toInt();
    if (n >= 1 && n <= 12 && token.substring(1) == String(n)) return (uint8_t)(0x39 + n);
  }
  return 0;
}

static bool parseShortcut(String combo, uint8_t &keyCode, uint8_t &mods) {
  combo.trim();
  combo.toUpperCase();
  if (combo.length() == 0 || combo.length() > 44) return false;
  mods = 0;
  keyCode = 0;
  int start = 0;
  while (start <= combo.length()) {
    int plus = combo.indexOf('+', start);
    String token = plus < 0 ? combo.substring(start) : combo.substring(start, plus);
    token.trim();
    if (token.length() == 0) return false;
    if (token == "CTRL" || token == "CONTROL") mods |= 0x01;
    else if (token == "SHIFT") mods |= 0x02;
    else if (token == "ALT" || token == "OPTION") mods |= 0x04;
    else if (token == "WIN" || token == "GUI" || token == "META" || token == "CMD") mods |= 0x08;
    else if (token == "RCTRL") mods |= 0x10;
    else if (token == "RSHIFT") mods |= 0x20;
    else if (token == "RALT") mods |= 0x40;
    else if (token == "RWIN") mods |= 0x80;
    else {
      if (keyCode != 0) return false; // Only one non-modifier key per shortcut.
      keyCode = keyCodeForToken(token);
      if (keyCode == 0) return false;
    }
    if (plus < 0) break;
    start = plus + 1;
  }
  return keyCode != 0;
}

static void sendMedia(const String &action) {
  if (action == "volUp") keyboard.tap(MEDIA_VOLUME_UP);
  else if (action == "volDown") keyboard.tap(MEDIA_VOLUME_DOWN);
  else if (action == "mute") keyboard.tap(MEDIA_MUTE);
  else if (action == "playPause") keyboard.tap(MEDIA_PLAY_PAUSE);
  else if (action == "next") keyboard.tap(MEDIA_NEXT_TRACK);
  else if (action == "prev") keyboard.tap(MEDIA_PREV_TRACK);
}

static bool executeAction(const String &id) {
  const EventDef *ev = findEvent(id);
  if (ev == nullptr || !keyboard.isPaired()) return false;
  String mode = getMode(ev->id);
  String value = getValue(ev->id);
  if (mode == "none") return true;
  if (mode == "media") {
    sendMedia(normalizeMedia(value));
    return true;
  }
  if (mode == "text") {
    if (value.length() == 0) return true;
    keyboard.print(value.c_str());
    return true;
  }
  if (mode == "shortcut") {
    uint8_t key = 0, mods = 0;
    if (!parseShortcut(value, key, mods)) return false;
    keyboard.tap(key, mods);
    return true;
  }
  return false;
}

// ----------------------------- HTTP handlers -----------------------------
static void handleRoot() {
  server.sendHeader("Cache-Control", "no-store");
  server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

static void handleConfig() {
  String json = "{\"events\":{";
  for (size_t i = 0; i < EVENT_COUNT; ++i) {
    if (i) json += ',';
    json += "\"" + String(EVENTS[i].id) + "\":{\"mode\":\"";
    json += jsonEscape(getMode(EVENTS[i].id));
    json += "\",\"value\":\"";
    json += jsonEscape(getValue(EVENTS[i].id));
    json += "\"}";
  }
  json += "}}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", json);
}

static void handleState() {
  const bool connected = keyboard.isConnected();
  const bool paired = keyboard.isPaired();
  String json = "{\"connected\":" + String(connected ? "true" : "false") +
                ",\"paired\":" + String(paired ? "true" : "false") +
                ",\"clients\":" + String(WiFi.softAPgetStationNum()) +
                ",\"freeHeap\":" + String(ESP.getFreeHeap()) +
                ",\"ip\":\"192.168.4.1\"}";
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json; charset=utf-8", json);
}

static void handleSave() {
  bool anyField = false;
  for (size_t i = 0; i < EVENT_COUNT; ++i) {
    const String id = EVENTS[i].id;
    const String modeArg = id + "Mode";
    const String valueArg = id + "Value";
    if (!server.hasArg(modeArg) || !server.hasArg(valueArg)) continue;
    anyField = true;
    String mode = server.arg(modeArg);
    String value = server.arg(valueArg);
    mode.trim();
    value.trim();
    if (!isValidMode(mode)) {
      server.send(400, "application/json", "{\"ok\":false,\"message\":\"Неизвестный тип действия\"}");
      return;
    }
    if (mode == "media") value = normalizeMedia(value);
    else if (mode == "none") value = "";
    else if (mode == "shortcut") {
      if (value.length() > 44) value.remove(44);
      if (value.length() == 0) {
        mode = "none";
      } else {
        uint8_t key = 0, mods = 0;
        if (!parseShortcut(value, key, mods)) {
          server.send(400, "application/json", "{\"ok\":false,\"message\":\"Непонятное сочетание клавиш. Пример: CTRL+SHIFT+S\"}");
          return;
        }
      }
    } else if (mode == "text") {
      if (value.length() > 180) value.remove(180);
    }
    const String mk = prefKey("m", EVENTS[i].id);
    const String vk = prefKey("v", EVENTS[i].id);
    prefs.putString(mk.c_str(), mode);
    prefs.putString(vk.c_str(), value);
  }
  if (!anyField) {
    server.send(400, "application/json; charset=utf-8", "{\"ok\":false,\"message\":\"Настройки не получены\"}");
    return;
  }
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
}

static void handleTest() {
  if (!server.hasArg("id") || findEvent(server.arg("id")) == nullptr) {
    server.send(400, "application/json; charset=utf-8", "{\"ok\":false,\"message\":\"Неизвестное событие\"}");
    return;
  }
  if (!keyboard.isPaired()) {
    server.send(409, "application/json; charset=utf-8", "{\"ok\":false,\"message\":\"Сначала подключи MiniMacro Lite в настройках Bluetooth\"}");
    return;
  }
  const bool ok = executeAction(server.arg("id"));
  server.send(ok ? 200 : 400, "application/json; charset=utf-8",
              ok ? "{\"ok\":true,\"message\":\"Команда отправлена\"}"
                 : "{\"ok\":false,\"message\":\"Проверь сочетание клавиш\"}");
}

static void handleReset() {
  setDefaultSettings();
  server.send(200, "application/json; charset=utf-8", "{\"ok\":true}");
}

static void configureWebServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/config", HTTP_GET, handleConfig);
  server.on("/api/state", HTTP_GET, handleState);
  server.on("/api/save", HTTP_POST, handleSave);
  server.on("/api/test", HTTP_POST, handleTest);
  server.on("/api/reset", HTTP_POST, handleReset);
  server.on("/favicon.ico", HTTP_GET, []() { server.send(204, "text/plain", ""); });
  server.onNotFound([]() { server.send(404, "text/plain; charset=utf-8", "404 — MiniMacro Lite"); });
  server.begin();
}

// ------------------------------ Encoder I/O ------------------------------
// Gray-code transition table; four valid transitions make one detent.
static const int8_t QUAD_TABLE[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};

static uint8_t previousAB = 0;
static int8_t encoderAccumulator = 0;
static bool previousButtonPressed = false;
static bool stableButtonWasPressed = false;
static bool rawButtonPressed = false;
static bool buttonDebounceInitialized = false;
static uint32_t rawButtonChangedAt = 0;
static bool longPressFired = false;
static uint32_t buttonDownAt = 0;
static bool pendingClick = false;
static uint32_t pendingClickAt = 0;
static constexpr uint32_t BUTTON_DEBOUNCE_MS = 18;
static constexpr uint32_t LONG_PRESS_MS = 650;
static constexpr uint32_t DOUBLE_CLICK_MS = 280;

static uint8_t readAB() {
  return (uint8_t)((digitalRead(PIN_ENC_CLK) ? 2 : 0) | (digitalRead(PIN_ENC_DT) ? 1 : 0));
}

static void onEncoderDirection(int direction) {
  const bool held = stableButtonWasPressed;
  if (held) {
    executeAction(direction > 0 ? "rightHold" : "leftHold");
  } else {
    executeAction(direction > 0 ? "right" : "left");
  }
}

static void pollEncoder() {
  const uint8_t currentAB = readAB();
  const uint8_t index = (uint8_t)((previousAB << 2) | currentAB);
  encoderAccumulator += QUAD_TABLE[index];
  previousAB = currentAB;
  if (encoderAccumulator >= 4) {
    encoderAccumulator = 0;
    onEncoderDirection(1);
  } else if (encoderAccumulator <= -4) {
    encoderAccumulator = 0;
    onEncoderDirection(-1);
  }
}

static void pollButton() {
  const uint32_t now = millis();
  const bool rawPressed = digitalRead(PIN_ENC_SW) == LOW;

  // Debounce the mechanical switch before interpreting click/hold gestures.
  if (!buttonDebounceInitialized) {
    rawButtonPressed = rawPressed;
    rawButtonChangedAt = now;
    buttonDebounceInitialized = true;
  } else if (rawPressed != rawButtonPressed) {
    rawButtonPressed = rawPressed;
    rawButtonChangedAt = now;
  }
  if (rawButtonPressed != stableButtonWasPressed &&
      (uint32_t)(now - rawButtonChangedAt) >= BUTTON_DEBOUNCE_MS) {
    stableButtonWasPressed = rawButtonPressed;
  }
  const bool pressed = stableButtonWasPressed;

  if (pressed && !previousButtonPressed) {
    buttonDownAt = now;
    longPressFired = false;
  }

  if (pressed && !longPressFired && (uint32_t)(now - buttonDownAt) >= LONG_PRESS_MS) {
    longPressFired = true;
    pendingClick = false;
    executeAction("hold");
  }

  if (!pressed && previousButtonPressed && !longPressFired) {
    if (pendingClick && (uint32_t)(now - pendingClickAt) <= DOUBLE_CLICK_MS) {
      pendingClick = false;
      executeAction("double");
    } else {
      pendingClick = true;
      pendingClickAt = now;
    }
  }

  if (!pressed && pendingClick && (uint32_t)(now - pendingClickAt) > DOUBLE_CLICK_MS) {
    pendingClick = false;
    executeAction("click");
  }
  previousButtonPressed = pressed;
}

// -------------------------------- Setup ----------------------------------
void setup() {
  Serial.begin(115200);
  delay(150);
  Serial.println();
  Serial.println("MiniMacro Lite starting...");

  pinMode(PIN_ENC_CLK, INPUT_PULLUP);
  pinMode(PIN_ENC_DT, INPUT_PULLUP);
  pinMode(PIN_ENC_SW, INPUT_PULLUP);
  previousAB = readAB();

  prefs.begin("minimacro", false);
  ensureSettings();

  WiFi.mode(WIFI_AP);
  WiFi.setSleep(false);
  const IPAddress ip(192, 168, 4, 1);
  const IPAddress gateway(192, 168, 4, 1);
  const IPAddress subnet(255, 255, 255, 0);
  WiFi.softAPConfig(ip, gateway, subnet);
  const bool apStarted = WiFi.softAP(AP_SSID, AP_PASS, 6, false, 4);
  Serial.printf("Wi-Fi AP: %s\n", apStarted ? "started" : "FAILED");
  Serial.printf("SSID: %s | password: %s\n", AP_SSID, AP_PASS);
  Serial.printf("Web UI: http://%s/\n", WiFi.softAPIP().toString().c_str());
  configureWebServer();

  keyboard.setTapDelay(28);
  keyboard.setKeyGap(22);
  keyboard.begin();
  Serial.println("BLE HID advertising as MiniMacro Lite");
  Serial.println("Ready.");
}

void loop() {
  server.handleClient();
  pollEncoder();
  pollButton();
  delay(1);
}
