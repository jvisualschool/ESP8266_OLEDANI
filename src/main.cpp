#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "animation_idea.h"
#include "animation_line_chart.h"
#include "animation_meteor_rain.h"
#include "animation_social_media.h"
#include "animation_bar_chart.h"
#include "animation_book.h"
#include "animation_calendar.h"
#include "animation_cloud_network.h"
#include "animation_fingerprint_scan.h"
#include "animation_home.h"
#include "animation_hot.h"
#include "animation_in_love.h"
#include "animation_laptop.h"
#include "animation_location.h"
#include "animation_map1.h"
#include "animation_map2.h"
#include "animation_monitor.h"
#include "animation_photo_camera.h"
#include "animation_suitcase.h"
#include "animation_worldwide.h"

// OLED 설정
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_SDA 14
#define OLED_SCL 12

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
ESP8266WebServer server(80);

// 전역 변수
String currentMsg = "Mini Billboard Online";
int currentAnim = 0; // 0: IP Screen, 1: Idea, 2: Line Chart, 3: Meteor, 4: Social
bool hasReceivedCommand = false; // To track if we should show IP or animation
unsigned long lastUpdate = 0;
int xPos = SCREEN_WIDTH;
bool blinkState = true;
int charIndex = 0;
float scale = 1.0;
bool scaleUp = true;
unsigned long statusMillis = 0;
bool statusDot = false;
unsigned long lastAnimSwitch = 0;
bool autoplayMode = false;

// --- HTML 소스 (Premium Design) ---
const char* HTML_CONTENT = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>ESP8266 OLED AniPlayer</title>
    <style>
        @import url('https://fonts.googleapis.com/css2?family=Montserrat:wght@300;500;700&family=Orbitron:wght@500;700&display=swap');
        :root { 
            --primary: #00f2fe; 
            --primary-dark: #4facfe;
            --bg: #090a0f; 
            --card-bg: rgba(15, 20, 35, 0.7);
            --text: #e0e0e0;
        }
        body { 
            margin: 0; padding: 0; 
            background-color: var(--bg); 
            color: var(--text); 
            font-family: 'Montserrat', sans-serif; 
            height: 100vh;
            display: flex; align-items: center; justify-content: center;
            background-image: 
                radial-gradient(circle at 15% 50%, rgba(79, 172, 254, 0.15) 0%, transparent 50%),
                radial-gradient(circle at 85% 30%, rgba(0, 242, 254, 0.1) 0%, transparent 50%);
        }
        .glass-card {
            background: var(--card-bg);
            backdrop-filter: blur(16px);
            -webkit-backdrop-filter: blur(16px);
            border: 1px solid rgba(255, 255, 255, 0.08);    
            border-radius: 20px;
            padding: 40px 30px;
            width: 90%; max-width: 420px;
            box-shadow: 0 30px 60px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.1);
            position: relative;
            overflow: hidden;
        }
        .glass-card::before {
            content: '';
            position: absolute;
            top: 0; left: 0; right: 0; height: 3px;
            background: linear-gradient(90deg, var(--primary-dark), var(--primary));
        }
        h1 { 
            font-family: 'Orbitron', sans-serif; 
            font-size: 1.4rem; 
            color: #fff; 
            text-align: center;
            margin: 0 0 35px 0; 
            letter-spacing: 1.5px;
            font-weight: 700;
        }
        h1 span { color: var(--primary); }
        .input-group { margin-bottom: 25px; position: relative; }
        label { 
            color: #aaa; 
            font-size: 0.75rem; 
            font-weight: 500;
            text-transform: uppercase;
            letter-spacing: 1px;
            margin-bottom: 10px; 
            display: inline-block; 
        }
        input[type="text"], select {
            width: 100%; 
            padding: 14px 16px; 
            border-radius: 12px;
            background: rgba(0,0,0,0.3); 
            border: 1px solid rgba(255,255,255,0.1);
            color: #fff; 
            font-size: 1rem; 
            font-family: 'Montserrat', sans-serif;
            box-sizing: border-box; 
            outline: none; 
            transition: all 0.3s ease;
            appearance: none;
        }
        select {
            background-image: url("data:image/svg+xml;charset=US-ASCII,%3Csvg%20xmlns%3D%22http%3A%2F%2Fwww.w3.org%2F2000%2Fsvg%22%20width%3D%22292.4%22%20height%3D%22292.4%22%3E%3Cpath%20fill%3D%22%2300f2fe%22%20d%3D%22M287%2069.4a17.6%2017.6%200%200%200-13-5.4H18.4c-5%200-9.3%201.8-12.9%205.4A17.6%2017.6%200%200%200%200%2082.2c0%205%201.8%209.3%205.4%2012.9l128%20127.9c3.6%203.6%207.8%205.4%2012.8%205.4s9.2-1.8%2012.8-5.4L287%2095c3.5-3.5%205.4-7.8%205.4-12.8%200-5-1.9-9.2-5.5-12.8z%22%2F%3E%3C%2Fsvg%3E");
            background-repeat: no-repeat;
            background-position: right 16px top 50%;
            background-size: 12px auto;
        }
        select option { background: #1a1f35; color: #fff; }
        input[type="text"]:focus, select:focus { 
            border-color: var(--primary); 
            background: rgba(0,0,0,0.5);
            box-shadow: 0 0 15px rgba(0, 242, 254, 0.15); 
        }
        .btn {
            width: 100%; 
            padding: 16px; 
            border-radius: 12px; 
            background: linear-gradient(135deg, var(--primary-dark) 0%, var(--primary) 100%);
            color: #000; 
            font-weight: 700; 
            font-size: 1rem;
            border: none; 
            cursor: pointer; 
            font-family: 'Montserrat', sans-serif;
            letter-spacing: 1px;
            transition: all 0.3s ease; 
            box-shadow: 0 10px 20px rgba(0, 242, 254, 0.2); 
            margin-top: 15px;
            position: relative;
            overflow: hidden;
        }
        .btn::after {
            content: '';
            position: absolute;
            top: 0; left: -100%; width: 50%; height: 100%;
            background: linear-gradient(90deg, transparent, rgba(255,255,255,0.3), transparent);
            transition: 0.5s;
        }
        .btn:hover { 
            transform: translateY(-3px); 
            box-shadow: 0 15px 25px rgba(0, 242, 254, 0.3); 
        }
        .btn:hover::after { left: 100%; }
        .btn:active { transform: translateY(1px); }
        .status {
            margin-top: 25px;
            font-size: 0.85rem;
            color: var(--primary);
            font-weight: 500;
            text-align: center;
            min-height: 20px;
            transition: 0.3s;
        }
        .anim-grid { display: grid; grid-template-columns: repeat(4, 1fr); gap: 8px; max-height: 280px; overflow-y: auto; padding-right: 4px; }
        .anim-grid::-webkit-scrollbar { width: 4px; }
        .anim-grid::-webkit-scrollbar-track { background: rgba(255,255,255,0.05); border-radius: 2px; }
        .anim-grid::-webkit-scrollbar-thumb { background: var(--primary); border-radius: 2px; }
        .anim-card {
            background: rgba(0,0,0,0.3);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 10px;
            padding: 10px 4px;
            text-align: center;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .anim-card:hover { border-color: rgba(0,242,254,0.4); }
        .anim-card.active {
            border-color: var(--primary);
            background: rgba(0,242,254,0.08);
            box-shadow: 0 0 10px rgba(0,242,254,0.15);
        }
        .anim-img { width: 36px; height: 36px; border-radius: 4px; display: block; margin: 0 auto 4px; }
        .anim-card span {
            font-size: 0.52rem;
            letter-spacing: 0.5px;
            color: #aaa;
            font-weight: 500;
            text-transform: uppercase;
            display: block;
        }
        .anim-card.active span { color: var(--primary); }
    </style>
</head>
<body>
    <div class="glass-card">
        <h1 style="font-size:1.2rem;">ESP8266 OLED <span>AniPlayer</span></h1>
        
        <div class="input-group">
            <label>DISPLAY TEXT</label>
            <input type="text" id="msg" placeholder="Enter message here..." maxlength="32">
        </div>
        
        <div class="input-group">
            <div style="display:flex; justify-content:space-between; align-items:center;">
                <label style="margin:0;">ANIMATION STYLE</label>
                <label style="margin:0; font-size:0.65rem; color:#fff; display:flex; align-items:center;">
                    <input type="checkbox" id="autoplay" style="width:auto; margin-right:5px; accent-color:var(--primary);"> Autoplay All
                </label>
            </div>
            <div class="anim-grid" style="margin-top:10px;">
                <div class="anim-card active" data-anim="1" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/v39/fv+/vz8/Pv7+/T29sPm5rvExErQ0IeHh1NTUywsLBQUFAICAiwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKiQgwGABAAkYPHjQYIHFBQ0mMkAA4GFBAQQUAggQQIECAgEMAFjwgOMAkQNfIniwAICBAARMkhS5E4CAAQUycjQQ0qRPowRUzmxQYEDDkQFgJlTJMsEAAwOyajVQIAFNm1LDErx6IGOCAwe8NjiAVazbgwJ+On36NiGBAwLjHnCwtqPCh2UdHIgr8EBIt1Rb+j2gAO/hkIw5PpxZU2Vdg4kPDDAJlPOBr5YvI0ycwKfBhl4rixYrwIBrA3RXy5791unAAwgQ4M37UnaA3CMFPmS5UeDV170PSKzpMWruqFKjxjXA16/Uhw0cwG4IHTEABg4ecv/vSRCn9I4OGICNDiAnx7gFqhdoiMDkAZKMFbzHHp5w/ZDdhfVQAg7Q1JRJPyFYAEsOlOYRbQC8pBYDFFZooVkRQhgTAAg4kJ6FIHrokoYD0QceRQ2kmOID6b1HIlwAHLAAhQvgFduLOOao444vFhDaQFzxKJxAERU4EYsMlGbdi4nRtBtBMk6k2lsk5aRAcAM+wMBDsJ2mUgESORicSSgFaFCApK2nkEqpqRmcSGgNmVprYcFWlXVxJuQcAiSpJJGNYTX0mXo3/caRmQURgBJYf5o2UIABCqolWDgdpqeVfs15U3547ZQfn3Y+ICaZiK65kqg2OWeUACYdyiZoszlk1aED75V0JasK4MShiD9pOFlGeBkFgFGfNSAZjsOxaKSHEzHLHI6ZKfTZlBrqVaCH2GZ77WA3zhZtQtO6SZusBaporoosItArjk95pSSRqDq6o0wZMWtshkJuWFBv+fbrr0EBAQA7">
                    <span>IDEA / BULB</span>
                </div>
                <div class="anim-card" data-anim="2" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////3///7+/v39/fz+/vv7++36+ujq6qvn54PW1izLy5ycnF6Xl1VVVRwdHQICAiwAAAAAMAAwAEAI/wABCBxIsKDBgwgTGhwAYMECAAIgFhAwoAAAiwgcPNj40OLFAgwHDDjwwMGCBg86IrTYwIFAhggaMLAYMWICBQACXGyoscEBAAN0GlCAAAABhSxdQgQwtOjRnABuQjUKAIECA1CFEqWaMClBAlsLJkhg0CpWgk0Vql3Ltq3btwIpRlSb1OKClEw1przrAGUDvyljbjS5QG8Dil0BtIzLdOtTmzh1UjWbtbFTpIqVRkz7VKdUyUcp69R6Ga7p06hTq17ttqJahgNhI5x7gCbmxREPtGzQEeXDhg8aeESw4OdFlMYduJRdMClDkj7vCi9gcexOkg7sphRAvemApyszM/8emkDA46iRBRIoYDYoVM63NVvm6jn95KuV4SdevFR//amh4TfafOAd5FV/jgn0H2hVCfhegvttJOGEFFZo4YUPMLATXcA58BNiCHB0kQDcaaShRSyVNNFcrLXo4oswxiiji9yBtJZsHq32nEM/MUcQTHo95CNBKb7UHAAHKIdARh5uKBB1wCXXgJNHtkRiAX5pdNJGUyrGJZYSOqSXSb5RSWRm2hnnl0CFmXRRTwKRpGGcCpBVoEFeRXTXYD1yd1MBAWjXV0kHfEdgfHF9ByFkU2GkwFERkcZVePxttih6ADaI1YD6USqffowyKNqDpXk6HoQLqqdpfhCaimBpqd6Itympkxoo3qv0YSqqg5LeWeWnqOqq6qi9xicSSN6B9N0AN4l07ADtiUTAAMkOwOKvB2SbbZ3aaquAAt1me1O4B1iVgLY5FrSjQ+y26+678MbbI0IVUQelsvaiqK+93H1k74j2DklvQzKtmNtf83J3kpBAEbzARKbVW1CN137k478zZqzxxqoFBAA7">
                    <span>LINE CHART</span>
                </div>
                <div class="anim-card" data-anim="3" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/vz+/v39/fr8/PX19dvu7o/k5LjKyinJyaWlpXV2dkVHRxwcHAEBASwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKlQYYCEAAgUKGACw4EGChw4HVnzQAIECBAMCNARgIEEBAAcUKJgY4CSDBzBhOmhAgABCAQIIOHDAIMFGBwAapkRwcoBABCwb2qxo4CWDAyAXChAIkcABBw8YABDgEQGAkCUXiOU482MBAiMzAiggoOJFgW0fLKAKl+SCtwcaZF2rtq/fvwkFsBVotOuAnATT1tSbYGJTrHIxFlyaVaLNlw4S5DSacuXXll2/DsxJoIGDiVNvpoZrM2jIwh8/G+RsNqhCnFflJmhMwIDeBzw1Tz2ZILKAphwPbP1boDHBBFgvtgZMvbr16w5XYxcdOu1AmwkYiP/fudMvgeMMDMAGKSBASZu0vQ6wWV4g1rnLJ5cGPvNA8awLGIATYQJ0NpFRCTEw13QGnVSRWFqZRhVtChQVkkAnKXbZXgwmdJZNEgkV1QDr2TaQg1kNVYBICS112lZTTeQaAKENcFZEAJg2kYom3oaSWHe9tMBLXQmmFAFOJaAcdDA1IF1fLskFUX6YaXaeYDvpldleJ/k11QExxdSAW5KNdtZUA2J3HkEdbufmm3DGKSebcpKoFnhwFuaVdwj9x8Bxbfa1nkjeNYRiTD5x1FN+gtL4EYsEGVWAUTZd5UBEx+nVwAFp6jelXnMZ1hIAxR0glEqmngRmAwxsOtNa2o33JkCrDWj6Umw9DmTgZw6OaUBeHCF2UKUzdVnAAr81gBGFBwaFgFc35pSajKoZsJdALzWAGoVFnSrfcog5VRNC5/3WpGnnxVeUQWkJCIBTsAZGErV5NeCthY4qh+CXQ6b4KJ8GdRpoACWOhNh5PvEIcHb5EeyofA3h9Ct3tS08GakMnOSgA8rZVGNBGylZsXVaKlsjeGFyrNeifkUZprLFdVRQjDtN1OVLFgnrEMpa1UWAT1g5cGWUvxpQAEc2BXpbbxxJdBaw6S2HMnAMQPZnl+blyDGyWrk1JUJd3sXonaRS3RjOFpXpodItn6dqgGPPGdhgcgMQEAA7">
                    <span>METEOR RAIN</span>
                </div>
                <div class="anim-card" data-anim="4" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/vz+/v39/fz8/Pv7+/b399Tp6cXFxULKypiammVnZzU1NRkZGQICAiwAAAAAMAAwAEAI/wABCBxIsKDBgwgTJiQgAMGDBgkWOHBwQIAAhQUvImDAEQGAixgLEiggQIECjwEGFBAJYEAAAAhMGgQZUiABAAkgArgpgIABBg8ZOHiwAMBKgSsTPEhgtKbAiwcWMFhwAOjQBg4MADBgoEABrlsdNHjgwOvTm053skyrFsHYh0zRsiUgl61dkQ3FRpyoFcCBmCYDm0RwoGUAh0wLK/VYV+FLkwJersRaWKDSon0DlFQA4OXdrQAWLAUtmoFBoJhxEgVd8yZiv6oZH/WboHaCyn4FiK5MwIHpvp+DCx9OfCBaBB4/fvbJtXFCrQ0a+C3c4HdKzwSv2wQQvbCB6GpDgv8kHPpBYAQ0jQNWEJXsQ7LonSd8eUDmALWiH+h3ELfzgM2FYcfWfTF5dN9NWH2klUO/CaSVaEzNlhZUDzTYU2hD6UdWUQyBBlRh6dUkAFeFiVYUhCMSZIAAl5VXlIQhIeiARwiINZBEZE1Vm1RDOVAUUG/pJB9eSok1UQIXEiSAV17R9OADuPnGWnGtoZUklVhmqeWWVHb40ZBcbpeTfg0wZldjYKoIgFADHbAacCkNIKeAO7E4EVb8XYmQVkKB5GZqAv2FHHkCDeDaaIo9YKZCN435HgMFrJfAoAgkcB5IWHl0AHhpEuQlAQMMNhBXXBVw30eAVRjVVCDaReBJLbXUJRVVaqUE00kjchViTa8amNRDhCUwUUUX9RrrXfQpwNSpANhogE8EVMjafZa26uqtBpK2ml9A9deUUhHe9SugF+ZX4UpOlhfuXTJ+tOSaDy2A3AJjmUbSRTZ2mpBFBzjrpnRf0dUVd1BCR5FFdu06VkSKIjyQRQ4toNRv+nqal3tDUQXUaAcp9V5Rfz0VI0yIxvZuVG9pGG9lBkwkcaYOLyRAdFptKp2VK860Ip/SHUCddMDtCxNHDHi0674C9AvRmPE5xdyzn42X3NFhilRx1VhvGRAAOw==">
                    <span>SOCIAL MEDIA</span>
                </div>
                <div class="anim-card" data-anim="5" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/vz+/vv+/vz8/PX5+dj19b/v75jf32jZ2T7PzyTIyGlpaRcXFwAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTJgwwIMBAAQIUSpxIcAAABQsAGDiA4MAABAwOAIhYYKABAwNLAnBoIAGDly8XIFh50CJGgQ4tghQJoICDBhEfNnCg8gCDmRYHLlAAIGnFiwsIFHQpMoCAnwaHCnBodCZBAkubUhxLtqzZswktuvQq0ICCkDTRUrS5wGFOADsF+nRg0AFRgUZfKhi8QLDYgnSf5u2JVWXJoUWPHhYY1qnBoAQDOCSoknNBzZflih5NurRptA4JJMC4YOnMzadrAnC5IAGC22/hYhYIEaHV0Ajpah4wgECAvAUEJPDboPlQBwkElDSa4DMDppYFCqepEy5jv+DDR/9mqxS7wcTc8Xr32aCnXgCQAUu2XPk8VNjdebJ37z4+gK6TBVCfQQ4hUBhM16FkVQEFPBdeAwxupVFuMMkUF0KPgSdSZzgNwJoCDRU0HXjtcRjbiSimqOKKLLZ4kGawuTgQbAQYsFmMK2rm1nUJrFZYdbulaBMDKBFE1UhmMUQcjvZhRCEDS+mmF4cMHmTAbR3hFBxUUhGUV28HOPiTSBEFQMBbth1wQAKFkdlkXekhB4BfPAHml3tGKVAnASCZh9h9cXpnAIk9OlgkdWy2poCBfj6FkV1iLbbfmgmI5F9XxfE2wICOZhSoflgZwCBKl85HEKcDoaedeqCW+F6pSJ2E2miqUCm2XmOv/vWfqeVNRutNDZn50a0QilqAqLAWNxwAqPKmnkwHbOSSAsZl2EClaw5VoploRstRYTMFOWNTHKWpJUEtNZdAkeP+h0CaFjFJoEYGZEcQRCf1FtxJFypUUgLOuZlZAKspkABo9/7nXHUmJsRghGkRZ+9DDzcs48UYuxgQADs=">
                    <span>BAR CHART</span>
                </div>
                <div class="anim-card" data-anim="6" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/v39/fz8/Pv7+/b39+fn59HU1KTBwZiamnZ2dlRUVDExMQ0NDQAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTGiwAgIEDAwcQIEjggIGBiQsYNHDwoOMCAAk0NmjAoKTJBSgXHEDIUMEDBAIRPHDwEICAggMAyFSg8wHPgQserGzoIGdFAAQE3jzw4GMBhgleIjV4c6RABh2z0kxw0yVMhwxvAsiJVWuDpgfSpjVgAOiDtgQJPC1AICfTBQOiJpg6UAABsQohSkygAKVGmlkTOxjJIKhThAPkziVA+WldwAozH7xpoCkAhmcf7E2KU4DVnJsLCNALIGhbsUkdKhXQ+XFBhkGHRpyooPBeALVBiubbUqqAAw4b3MQ8dSdwtQYKBBgY28HnowzHAmD60+CAp2zZ9v90ALMggQOEDzsovDEpc83wMwuYH7++fc05EZRc0BtBRv4IHJDdQDlRFhlllc01F2qbATCSAE9t5xOEcwFmAEcWHSQXWwgg1hFMqOXnk0AHIAZiQTcVcNSFiXXkwAFdCeVgAzYNhJuMtCmwmHbmCbdXVAskkECAslVF412fFYQdSEHxNCB1RJGG4lMi8uQVj0k5l11uNRLEmWdP3taajKfZOCZBKjLw3QDztVkcTOdFJOScqB2lpVJjHUVQYSmlVBJHMwHqIklBItDWTgdwidpSh7Xo4mKN8SdkUwYwGJgBVklpX5sHTmZpQhC9d9+opJZq6qmonvodXZ+eeqOgjs7/RGgCK7WqkGUGJWUVQZ7dhh5WWC02Ek3EwtqApE8uh90A6I2Y7HcOvgicp5jtZICiAtnlGW2AwonQYvMV1tuQhibplQAO5QSbjzEFuhJzS3l2l5wKYFWeazPyaK5Uwj1Ao4Y98STTb1c5kJ1VwQ3IEFhdHqWpQG+CBGtHNM5np7ME5juAtrahOWZbYWaM5JWkpYjdcS6N5l2+OikwZFrZJcUal2JV+Zl0V3560wAOX0RYYSVxBcCVssXF7nHEvoaThLbNt2oB0DLssJm5RSaQVe/JPBx6c3a9cb5IDlhdgHDpuRABXsmFkMkMBJwkniMBrVGsgR7rENcBhiyAcznjUVlbeNEpdEBoDgQJ9EaJjaSY0s3529i4ZIds0wNtg8cW1AVBdHXFawv2c0bqxSrs4y6TbemSmwmwsWSWieorvYdvRJOTqabe6YKu16777qQGBAA7">
                    <span>BOOK</span>
                </div>
                <div class="anim-card" data-anim="7" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////3///7+/vz8/Pv7+/b29uDh4Z/S0rm5uZafn3p+fltbWz09PSIiIgsLCwAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTJhQwMMCBAwIICBhAkYBFAg8vahxAYMBDigIYKiQAIMEDBw4aqGzAoCWDBTBhKpipIIFNmwhy6jSwwAFJkQZJKnhQQKCBBwoG9hx4FMFAlgNNGgAwAADUqggZMmgAgCSCB04FsiRQoMBQBAbSPlhgICcDnwILIO2KsOrXB3jzpmS5oGbOtEUVDhBwlqrCw4gTH7YLFoDDAwc5FniYtnJbnQ9tMngwFWhBoZwBeFYclCyBraITklzK8GiCgagFfp0q0MECw0MDC7BNV+FV0Zd1IriZgCbNmDBfG33wmuTB1nNJFhiuAOZWB3n1Yn9ZE6rXxs5Ji/8fT7684wCkWzdA2eDAApYukS84ULwmccwHLC8G8NWpyJAUcaRRWRoNSABgBCxAVGoHkSTVQhGVVYBlaQmX000qMTTaQKDRJtdJ2YUoYnYOTEUSahtyCEBPzh01Iol7neRSA6Ex9FtdAKDGGHxv2WbdWvY9wABOPQUm123hGWQjA3R9pYBODjDQlgFDAXaUAmStyBV0SSVJkIa8ITThZcUt0NJ6I6aEXXM49ldQRBqlyOGECBTlYI1ZicZbUSahVQABYQKQW1xzCdSTSHJ1ad6ijDbqqEABoJdQpI++GRIA+TFIEHplOVapQBY59pBCoyZUamKMvTSfAmeuhBJK7r3/iNdMeTVQlJyMIUBWfiCN9hipkAm6oJzfhfXpig7citCd1s3oKkqyRtvAT8uWxBlHAm4UkpwJvXdYh6INUJ1MNBGXgIUUCqTVlqoJayRSEvZUmUlP5qTSTQqaaBWT3K62IABXVrYVYP1dCpVADy5pWLWHLrcAhg7QtNlLLZ30bGcA8OYlQVWhCHBjArU00GwDBTpoVxon1DFX361E40nYRYuXrYQqmqd3/IFs1W0I/5uoUg4wFd1C+zYpc5opPTCtRFf2pmRtC0REEXl02mQdjWw+BzWZ4143okrcnYuWAQUwpKBTG4Nq7cN/ka0QnBd5FpGwTmH1tGtk2f0ZndRdLh3zWgAU5abeXwY+F58kKt1XAm9ZqbOOH2etNc5uisWAaVLRKWSZ6wUGKJIFBQQAOw==">
                    <span>CALENDAR</span>
                </div>
                <div class="anim-card" data-anim="8" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/v39/fz8/Pr7+/f398fi4rm5uXqcnFJSUjo6OiYmJhkZGQgICAAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKiQ4AICCBwgEFihAgEABgQgUaGzwYMEBAA0XGiTgsMEAAgIUCjiQwIBKAgMaKABAUiTJAhkVLFDgoOOCnzwd6Py54IHQnQoQXKwpMiXLBB8tDlDQQKCAhj9BpgQgcwBFAE8/brU5kenAhgYeJBCY4IHLkAQtfhWZ8KpAAwjyugQgoO9YuioBGOD5oLBaABcJJpYIoK1hBy35irzYNmKBvwIDCHwaGTPfiwgOLwYskCRVq1gXaBUokybpgSkNOFhgwACDBnnzikUMIOtFp7kRNGBQe4EDl55fn2WovDlBARVROm8e+6HM3FQfKDhgYCzc6Z97V/8FeTCA17kHHDAYKB3wAAFFIyZsyNKlZoShF9ity1qBgNHLsXSAgAkkcNF9iglwmmQJkRQabntNhxdHEZnVlEUYUlVRWQwwIFdFMmFIUXILnReSaePZldV7/bkG0kTfIdSXAcMNWOOAC6SY2mpceTTgbT820B2JAjWUk2UTFdCQg0b1VOGLSQJwJHngsSelYRC5WOWWXHbpZYMxnvQlf1IWheUC8pXWXpdXGVBUZIo55sB1RXKJUmhrXVYQdAQZkMCcB3hFZGCCzYbAbiq51FYCCOw1KEMCuKndoQgwSiV9kWlmJKN5PUTbfg1emSdMUjFXXwAIehUdZVla+FyLrg7JVIBL9Q0Q4UgliYTVejFmFkABnBV4AKoHNcSAar0+F+lwtxZknmCBUnmQbUL2ZZOUDBjVwLYdbuttt94Od9u2PTHwJGm9LjhQVgS1VlCyr6GIWm/IbtVarODJy9eOLHI1E750fbdkrvvSy+O90iaMkIM9GZXmgirW2yJGDTtwbrFgHdYYlg+s19cAA3QIsrXZYrnWxh/B61pbDjh5kFPbIlpQaC0fBjBszClbwI23XYbZd48C1tfODATp85iBhcUg0iXCyPTTCQUEADs=">
                    <span>CLOUD NET</span>
                </div>
                <div class="anim-card" data-anim="9" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/vz+/v39/fv7++/z867q6tLR0aKiokzS0i28vGVwcDc2Ng8ODgAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTHhwA4MCCAxAPGCgooMBBAhgLFjDA0cBDAAwVDiQAwMCDBgAsMjiJAEDFkisbGCCZ4EGClCIRWqzZkoADBgJ54mzgQCACmwKJ5tQJQKiAn0Ef9BwJ4OhNAhxdLmXocIHXBQoABKBokSCBAmcFgGwIVoECsGsVCpgrYMDYpQjrDthrV25JBRPvFkTQkuDLrGoBkBSbUsEBrTkLHHjr1u1ktwsYfG37lrNXwIzx4hTqE2hZgSQboJxLILFogVwXTGSIgOjVnVJxrmy5O2lRl3BDvh5OvLhxvAMMRJQY+vXY5RADI4xtgK/guQYYOHjwYOJOBgjmDv8MQD6Ax8fCcwrA2KD74QQMJiIAL3Cng6kCyYsOsNcBygEF+IeTAQkk0BJuCfDHE4DHEQRVU1JVJIBSVSEFAIXE4dbTg0JZpJQA2c0EAAOrZQjhhkBhlQBJJBHWGgL3+QbZawgSpOFQDryIVAEbzagQdQOtB5laBC6m1nwNrCgkYVq5FVeDUEYp5ZRUVmnllTnFtpkCdgnmUkYJjdXZV+hNx9ZEIrko2nlPFhTbY65VlNpJBrbWWk0okeTaWGymt5BhJSVwWk5Y+bgfVSvdZN9JBghQ0wPxKWaAfzO5NpxKEUII1EFHNYDWhQ4UIN5wLSJ1JJoJbMcdA6KCalEBkCqv5lxjlWHW1meV4dqZraB5qZ55girGYW4e/maVb/pdeqKwm3YIqlEWYqgsaVAJMJ93z8aZkkWW4nVjaThR9SFORLbU7VLfDoufrMdeWCJe+uE2E1YiOovAihUq6qavFDWkgKggbsfAimct6yFK+QpEoqwQGToQdfz5ORqx1m4n8EtEBTCXkxLDxpZEHHEr0rkE8cUxQmNJBl1zoqk12cokj8fXXjHDO3NfWOasc0EBAQA7">
                    <span>FINGER PRINT</span>
                </div>
                <div class="anim-card" data-anim="10" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/v39/fz+/vz8/Pv7+/T3967p6ZbQ0DXMzIqKik9PTzIyMh0gIAUFBSwAAAAAMAAwAEAI/wABCBxIsKDBgwgBBFiYsCEAAwAWPJjIoKLFiw4eJAiQoOMBBQkAfEyAQIECBAAIFBww4EDGAwAGGBRQ4EADjQAKGIQocUFMhwIZPIApQCSCo0iRKtTJwIFDiQ55Dk0JIIGCggSsqmTqtOhOAEIheiVIk0GDBmbPql3Ldm3atm7PMiAKFKGAu3jzelVZEuZHBAFGUm0IUahPAzIRPHBwgGVBiAkeNLCKEgFMygDG5qw68YAAmUALP/DZUGhI0I8jTlVpdeHCAlkVbAXb9aBourZVw2R9dSDv2U0z5w4rvK7x48iTA1X5cbdygagTDigqk3mCAyVPklQYAKhMl4zB4v88UMCA+fIiHYQvmt0kzNCcGTwcK/XwV4nyDYAUrplg/YcN6RRZAwYU0B9N4tl33wNC0RUAARBGCKFXXGU00YUYYtjAT6lJ5JxVB4QoolabBdefQLcVt+CHvbFnlQIQcaWif+LhtiJVVhFEAEeylVjbV8SdSJBEY+VYUEkEBVfaAw4VwNKTUEYp5ZRUPmngc1hmqSVB3RnE0JbGqRTYSQKF+Fx00nH3kQIwWQaAe1/WxdNZ6tWp3oUOhKTTQC+6l5xOijHQU0IuaWQSm/uNVNJ2QgpEkwE3oWSATQ+gtOdAgC52GXYebbcjYaopCBmD86UWmQMojfRgjl0alGkDjfXQJ5qCNOJXFUqf2iWangh9px5MaAowK4AJSZUffLYWWMCyy5pngGnEEjRdoevVZaBQo3Wk7bYdZZTAZ2SJdICHMyak01kHcMutYYzuNAC5vB0qL4w+DqAss8uS1+CyCEnFokEkyihkio36i2NvBAVMW7ko1shww/BW1Rtfh8a48MDiidWvbgfrKDFwP6YW5MYRGynQbz4+LJrGtgngobACWJUXATHDCHNTn+l1lwEChEXTgQWYNZEDaqnXltFqTQTXWkqjyyGYUEct9dRUVz1QQAA7">
                    <span>HOME</span>
                </div>
                <div class="anim-card" data-anim="11" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/v39/fz+/vz8/Pb5+czy8rbd3a6urlHU1CjJyW9vbzk5OQ8PDwAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTHgwQ4MACBQQEFGyo4ACAAAIlDnDQAECBgR8ZPDAA4IDJAxgNEljJsKSCBTArDhSQMqQDiQAUKBBo4AEDjwMJKDSYEuHHBA8QCESwwIDEBg4ADADAoMFPnAglOkRJEioAAWAFGOAIVihTiySRJgA6dCDDAjoLHPUJdO5PtgKmInjQoMDUtgJbAjCrQHAAAkUHTj3AwMEDBwyUfgQ8+GLJBRYFbvWIQAECuQDULm0qsfHfwAgxGpApkYDnwapJBpDY86pDpaGTSqUcmGFcu2z9ehWokyfdjy8fylZoFvPuBh0/QhdYgOPSB2tzKz1NObHCyQAMkP/MiJW3+YMDwPs1H/siYaG0FSyXKuAAR9zhG+gucBhxQsEuwfSQRQytJNRix4FE1QMWnYSSSp19dllmE14UwGQhMUhcYU91BIB+HXFHEEYHVOSdAK5h1lKHXxmwAG57rTVAZ54JlZp/lr1mo2B6YacZZqWN9NV5gY03gADW/ZXheMUJZN1HR5aXWngPASUahgte5SJuV57X3HZjOeCUTU5l2ZiYSw7Jm0QvGYDgT2n5WIABIj22lnQhEmkAYiF1hJUAGfLFwKCONWCRiESaF1aiCnnX1qKMFkQAeEP9JdZkiAKGkVAvfQRgQVMlUGhVD/iYqUo5qfiWUzSt1pRgGVr/JNdHY+XZlmoPEnAAAgdQWhRtPgF6mEcCIKVUck1ZdhCKrh5ggEPyefdRjFIZgMCkC5IE1kLhvSQfAEztWQABW3ar3FwwkvahhwkwsJaURDX7rHIrEZRmTjt9BZlHz2r7n727OnuRwDWt+1UATR6Q4FAMufhqRHsy1FCVEemr5YsCiTaATjrNl9qmFCsr0JtfbRXkoVPRdN6e1FE6V3bhlubAZI39dOqUocpI3YKZ6bTtdNw5epBQDpE0FdA7NzZZk7UBRa4BlGYFrnMDhMnqyAYDijG4cpaEwNc2Sr1VAF1Fta1dUM/54gAB7NVAXl8ZyDCVO73skd1z1tmAZEjaW9qdbxAB15WH1bHro3a78YbYZkJNR5LSU4nkrgNiHimS0Wtexrd+fNdKrNckxXknb6ph/PgDBWwLlEh7E2QAUg4cAHek1PlL0GKkPtZAdjefpzp6LkdN+/BtBQQAOw==">
                    <span>HOT</span>
                </div>
                <div class="anim-card" data-anim="12" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////3///7+/vz+/vv7+/X7++/w8Mzw8IPm5qTS0gjKyqipqXZ2dkZJSRsbGwEBASwAAAAAMAAwAEAI/wABCBxIsKDBgwgTJhQgcIGDBw0WEFAI4ACDBxAPACgQQGCAARURFADQUeDEBQ8WABBwQIHIigpiyuQYIABDgQM+lhQgwADElQgTpAQwkSACBQeOIiX5ESYCAANAwlTAkSGBBwyIGpx40YCAk0NXGjBQ9CDKBFpNAmjwwABQilodYnU7sMBIk2UJyGUw8SZcil/9IiRws2RUnHdLGgzMVmWBnDn/DpxogC3aAgheKh7YwMFEkC2fSiUJ8miB0AMzJ43JkXDXtwUdNFhpFSsAAwweQlxAd+CAmC8jqz2bdisAlLMnCm3g9uvEAxAr2857/GdZyQS/EjRwIIF3sgMDY/8fT37g5s2SixpYwBaj+/cOGGi8rTopggP2NaIPf5st35WEAVaUXCpN1BFm+hWknHUhuTSfeXAt2AACbu0nkE9ZTRQTSRwKxNBRbmEmk0j4ecQThsVtNx1TBcT0EUgGHPVSSxqB9OIALT5F2YoFQZehVu01sBRRATJlgEtTaQYUitcNhNIC4jngAJF1JcBAe+5JGRFstMnGJUFcPXAZABdh1AADVz7kAFoBsnWAdgCu5YBXgg1GZnQCJRBkA3yayaZa1c35ZXoCWeTemQskukCaZqpEVJ3lLeQcT5BGaumllzpHwKacQlqTR4pZqBCctyWwKJoMJNqbACBhVl9m93X/OCplZca3QAJjFcDdokEmkNRdp+GUmageEnWRA44SSSmlm96E2wNrEvVpR7/KGl5PjcWJnXbSFbhRZjQZd5FjBUUVFbGT3SalfSNZyFBnXgGg1IiwtmuecGDe+edB7zrwmLwKdATjASOhJtVpd0EYpkaVVvfmSKZNC9kANAIAXGYyxaRfvzzx6+VnANcIlUAyAmzAuQShNpJQ3vY4VFEDCJAAkgC71NFHMcV8M2hIwsgjQSxrdRIDMwPH0GilKWCAwEptTNTPAzF5k2xlRrRqpzJqjFOx0LVc0GsMCSBbWVbqZiuuI901mlpgH+RTcmF61aRAp53aZ3KTyfwyvxWtZFiAbBbptttImxZAwNuwLfgjQidKafjhD6EVnkOC3ul1mF4nFGZWDKGUUm93YtSydsjRiV2Y0TbUGUbxSU6YehcxN+hftDYKeo+5Qetow9uqx57g75354NyYergp40UWr7ylAQEAOw==">
                    <span>IN LOVE</span>
                </div>
                <div class="anim-card" data-anim="13" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////r///7+/v/8/Pz8/Pv+/vv7++bq6mTU1Ly8vKmwsGxubkdISCgoKAwNDQAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKhwogIDDhxAjSpz4cOFAAgAYNFhYAACCjx89gkRgIGSCBwoAGFiIUSPBlScbHACg4AEDmA9kAsB4EcDJlCsJCgBwYAGDowwc2FygUAHSp1CRNkCpMqEAA1izat3KtStXi2DDik3Y0kGCs2jTqj0Lcu3ZmkBZZtx4cKhAuwUEHEBwIMDHmQUw/qyqsOXGlQt08mTI+KBgqkGFGmhg9oDXy12LorxqgMGDmXbHih5NurTp06WHHlDgtjVa1q4TRC48l2LEAQD0tsXdcMAAAoNnO547di9IlX8FBpfrsiDOnKCrJnDgYCbPx3FpN3+suOqCB9B3Gv/ETrjgVYcGCGjEajf0RQJYbcMHDrng4wTwOSfQ2KC/f/8MpORebsQJdEB9BqmmwAIMNujggxBGiNQCAqJm4YUYZqjhhhx26OGHIA7nGXgklmhiiQsodSKJCYinHV1iHXfQgdmRVeBCxo0E0kwGzLTci2DlqCNfyf1oY3N1EYBXR8cZx6NPCB4J40s+MWDdQHs5xRcAHZEn3Hg3CoRTAzXddBWU3QnkJXMwjvlkTRSGt5iaUNaIkGE7CVBTmt5Z6aJ9dZZ3Z5h31SXAgAStaRGSthUXJZ0nUUddf+A1EBVS4EmqKXWV+veoeINlaKl0D7Q4Z6EHTLXiqqyWqOkDTJ0MKlSItNZq6624JhQQADs=">
                    <span>LAPTOP</span>
                </div>
                <div class="anim-card" data-anim="14" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP/////+/v7+/vz+/v38/Pv9/fv7+/f8/Pf399Xp6XDc3JC4uCHHx2VoaB8gIAICAiwAAAAAMAAwAEAI/wABCBQooKDBgwgTKlxYcKBDAwAaPGCg4ACAAQ4zatyIEYACigkuOmgAAOJAiBIRABAgcADGAR8ZyKwo0IABAgRuXgSA4KOClitHlnwY8QGCASw1EojowMCABCATQGWQgKUDBytbIn1A0mTNoioFYEzwMeTQlAAKZLwpcQGAAx1ZEuA69CRYgTFDdrQrsW9djgCg0vV6lmuDw4gTK16c+Cpjv4RRGsVYwKfavRc7HiA7kyLQgWMHE5XIGWRHlwCWLnjANYHFAgkkPnC7dKzMBEIjF+X8c2dWA3QDfLQo8ADFALtjKggpmu8Dmphtzxy8NMEDBwhwSjcLvOto1uDDi/8fT748eO8aDTBcn9AlUoWEnSOwSb++/fv1CQSYmiCAgQM25fadAfz5thFoCEb3lk8CypfVdg6VtlxGUE0Yl0jofZVSR7ApZ9ZbAiWwQAMjNWAWAR35pFIBzWn4HHEGCoSTAw+ERMBmUh2wFAJXIdBQii0WRlWMWSFAl3E/qaXWRxCt9mFPDAQpmUoeUaQSRk3OJsB+MnXZHwCrmbhUWgPMlWFhYX3mE3FXZfVVmDBRZBFSqUm5222f+RZAAKstIACPNabmEEYHrGmnRBqtudFhBzYaZKOQRirpgUlNammjlQ6k2lVXmefpdddxCmqoV9H23aWRKppRgy72hhmqQEH/CR0ArBZW4KuQorZXRwXWOiVPPuWZ0V6c6TUoXlGe+ataVSpwpYFQggSXonuJRauydyW13WYeIrjTrW76ehdqIXqIUQDMZiQAigtSdJkAhxoFbJc0tVSdbKGy5oCN3+IZ70wf7lXddR9GSKOphH4U71IF4JolbVN5qdpsdXUU74oaySWUcAzAiGQAAqDVcHH/DokZS0aShGRaLDMJZqASxusUtTLS6oBaFSaAQLFiXXUbcSxiixaEqKU8FLeuXcRjU0DJdfGD7SowQABPUQyRe5I14NlOTgv9AMPH8hdboEtlSVJ1ILHc3V9CWnhsoSGlxBJu+wq6k6wlLVyonDG6RCSAAW22xbaaFF1sE5fH+UfffNbRNR9+9B2wX7wLVG75iIddrvnmnF8eJMpShS766KSXbnroaaqL0+qst+7667CvnmlAADs=">
                    <span>LOCATION</span>
                </div>
                <div class="anim-card" data-anim="15" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////v///7+/v39/fz+/vv7+/L29uXl5c7u7orl5ZnOzo6RkT7R0SXDw0dQUAAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKlzIsKHCAQAOPJjoQMGBgQYUEMy4wMHEjw8cNCggcMGDixATQpTogAGDBAkaIABAAICAgyQ9GgDgEgBJkygfRnywwCcABDIHIm2QQACBmDtjzrzpUwBQACkZCihQgGrBmwYSuHyJgEFSAGIdql3LNuHWrgi5DvA68C3blUSNEiDw8yTWgzdvpu0bVOXQoiSRzgQaYGyDlzsJxmzw2GNhhHgXHOgIcoFnBiAdVDSAlIECjxQ122zLmi7r17Bjy55Nu+EBBZxBfnSg2XXE3BMvH8wMwABchG8LQOwo0OVWAFezDj+MlXSCnSZ3uv7pgGRPwn8NS/9ETEDxb5IBCNYsmLakX+kGiSdOauCiAbMJEIid7Lcu9PdCjTeAceaZNFYCAkElQGMJ8EVQdAqBJdoCCkQmEFcCIJVAegVs9hgCT72U3gDK1abeejStNxkDFpro4oswxijjjDTWOCOJysnl0FvwLaTjQJtNaFFBcpGYVUaiabfQTbihFhKFQDUZmmZB7oYbgOLlVRdXAvFIUIceaZSWcRBmidhAEJ1GkmsCQDTAXIP9J1x8hzl4oZzhJRRnmZgdJhZlSRHQ4pcA6MQTA0bxOd14AdxknkEEUMUdSWI9pyid4xlV4EmNkRUZAW8KxoB+DFiW56J5zbcYp44BmkBHlOZ0hx2WfWaqKk02xYQgWg0cZdZFezl4aUHyHSXTAKdNlgBEChrAFE1bzTonsdQJhFRuLfF2kVgcviQQkh9d5BtBeE15kUCdSvnRY2ZNeC5bJA3042pLvfQni9YyNQCKao371UH3MaXrajYKtB5pBSvEb8KvBQQAOw==">
                    <span>MAP 1</span>
                </div>
                <div class="anim-card" data-anim="16" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////3///7+/v39/fz+/vz7+/Hy8p/a2qurq2mLi1dcXDk5OSwtLQ4QEAICAgAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKkQoYEABBQ8WICggcMABiAwMABiwsGNBiggcHBBg4IECggkeIABgwMFJih4RUoTYwIFNBw0aqARQAOSDmjYfPHBgkmdMowocaBQQgEBDACmXAjhwIEHVk0wrBmj5MiFHAwoWiGVAtizZBWbTMhiL1qxYsRo5Hj0ooOeAugXkzj3K8cBOABATGDCQckFevyth7vUKYIFShwQdFmDQwMDdxQO/OlhA9QDMkgkGNFgA0cABByupJrDqOO5CzV0HCmDp0uhAiohtC5ypdCPm38CDC+8oYLYBBArOCq44/KDPBEarJljJEUFRxcMpRgUQ0kDPni2hA/9Y67u5b8p5HwoVqsBy48oAZptHuRPmXa7mCwiwvpIAQcq1QRVRbwTxV9dRDZG1m2qrSXdAThpN1iBBBjTAQHEd6WcgdhyhFZ9ABATA0XZ6aajSgQhxpCBCp62EUAMnIaSgXgbVNcCNON5Y3I475ujjjznKN9+QRBZpJHDYEaTfkQLJlwBRoy2gk0muFTnTA+IZFFJGl81XnIIFgLVeRN4B1huNwYEk0lRY0pcYjLoJp90DGoXk2XfhCUReksB91SZgJh1AmEl38RennNy1SdFxCBzwoXXQ8YmZZHheJxlkKSFA6ZKTkqTAp6Bi1NNsV4K62moGPDWXcS6FCZ56leGWxcBPg9UKFnxCetRhA4oVQEABMEbFlXUHcFRAZ6OV59GcDFJlWgNWQdQmaFU5O2iky7KJwK+3XSUoAdWqRlV8ITJ7aI0swYmdrwo0ECKGLAVQ2oe7vUdRrgZRJOW9BiW7bgDE6macugkx61COeYF2MI4JY7nwjYfV5xybFrr11qxrvTUWWUOpVZZOjqLJ5Mgkl2zyUQEBADs=">
                    <span>MAP 2</span>
                </div>
                <div class="anim-card" data-anim="17" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7///7+/v39/fr6+ujv79bW1snv72DY2J2enjyxsWFhYUBAQCQkJAsLCwEBASwAAAAAMAAwAEAI/wABCBxIsKDBgwgTIhwAIAEDBxAjSpxIsSJEBgkAMDzI0MCDjAQcYExAsqRJBwsACDigAIGCli5fIiDQ8IEBjQY7fhTYgIHABA8KACiwUyHBBUFr3txIUGdGo1ARAl0aFSGBq1izXq3KtatXjgA8JhAwYAGDs2jTos0ooACCt3DjDhAwFWdBpxoJGChQYC9OvoADCxbMsC7TgXgFfFWo2HDCwwpRClzgoCrkxZgza9Y8d/NBsowFhGRA03NBAg8JCFB8N+zOkCkjp1wJszZcmo5bi9XY14Dv38B9ExhA/IDx48cLFLZpt6nrpw0akGSQ1CPGBSazk8ROXWhu57sFmP+N3mCBagLjyatfH52BeaXNEbt+Sb++/fv48TO/rFGAgQYPWCRRAwMKGNEDDVDllWIFLOAgUuUtAOCDCwjFmmkErabhhhxi6OGHIIYYIocklmjihlxtpNWKLLaYlUD8yQdUhBTWaOONNgKYEX94lRbVVXOdiNt+OT0HQEgIsqfeA7OxRN9bTw6pIHivSSabSiwZ9+QBb0kZH4xGovbAmGSWSWaT+bXk5WU9frhmkbsBQB2ZDRTQFoBm5mlmnfCxaSQAPf2UFFFPRQWUd0TqVhSAZzlQXYAM9KSWWj05iuiU8okVgI8DBTBAAIsREMB3mX6E3KmopqqqcaSCKdaqsMY/ymqidwlAXQIu5qqrAQ40QFxCDCaAI4UIOjjhsA4mYCFUorFYkJWUncbihV91VGBEBEaEKYbUMibit+CGG2JAADs=">
                    <span>MONITOR</span>
                </div>
                <div class="anim-card" data-anim="18" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////r///7+/vr+/vv7+/n4+Obz89vb25jk5EPR0bW1tVGxsWBjYycoKA4PDwAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKgQwIEGCAQMZPHDwoIEBAQIGZjygwEBBAQQWGiRggEEDBAEEChiAwCGClw5deiTIUaIDBQAkHgCQcSQABQ9whjSQwECBBwwEHkAKwCFDAC1TGqDItICDpCE/YmxQkQGDjgYFGGgZE4HHjCElFgCQ1sHaniJ5Epj7sWYDql0VKCgA9GtQtnEDCx4ssGyDkwgAGFDgleuDvBwf7CQIlKtQuAgJDAA6GWrRqUgl4mwJsWHiuUsZbDx8AGNYAKkBZySamCDZAbibPsx4mCfvBjwXhjxgubPAxV7xNvi6l7OAtTqDEyaMcS7m6dizC8Z8XSWB7tghtv9ccBjpzqsDrRNwsPzu44pCCSNggGBAAwXPsxowkJKA3fdXmQRcTky5htBYCBxGVQMLJFDbATPRNpaDvs3FgAMHOKAhcOCpBEBvIRXgoADoFUDAhZ6t5dlhyV1UgAASnXVQWxllZBpsjt2k21u6ZQQUThJxtVNWBaX1gAEEmFijZzFReFxLp8H211pLxYfQajcx5t5jyylwAJFyWXdUUmsZwBSYCmXEV2MAegWWRv+pBhJ6aGpXHV0EFWDXggZq5+efgAYqKHUjWTeoQgaU5hpjDMiY0FzfTbcSkwcUYGZSfeEJmER5vVlnQvtlZFIC5FFF0X0zKcUUAHoy9piVC03/6NACM+75HlJe9qSTRFh16N1PksEYH1llpYrche/dRyADL4LaZAJvukQQUU0iAOlAQGl4wIU8HrRRUHYtkBhxjzEwgABOeXYRRCQCl+GZj+bkwHdzORXdh8C1pGJLM/3IKrCwEjQbvLSNmSS+nkHkmUnv4bRWbx0auRNEtBGol4b7USheAi+mxmuVgL1GIlbBkaaYXh5B+RRtoh7ZsgOOFglwyBlBuZ+siQrEr0BWkXxpyK8dhiZdsroEE4UgAbvTWpxJF5YACuDF5VeVFmRpVj395hvCvgo03F2T+aclgF2mqqrDM3/qkwE5dmkcbFripWFIbYXU9UcF1YTs1F+ZH82rpfBmd+fQtnJp6k53Cw5SpHlyhPihgVWXOOSDBQQAOw==">
                    <span>CAMERA</span>
                </div>
                <div class="anim-card" data-anim="19" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////j///7+/v39/fv7+/r5+fD29uLh4YXW1rKxsYWVlWNjYzw9PRQUFAIAAAAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKkRIAMCCBwkOJJg4UYFFBwwADFjIEEACiAUYQOSIsIDHBw0SMGgAQEDChgscGPDoIIHChw1yNnDgAGWDjAIarmxp0KUBBwsANByIAEEAhAYOHECgYCSAAAgOECxw4KHWpQVdJliwUufOB2jTql2LlmfOngwMDHBJMmFQAnjxDtg7t+5CsWV/iky7oLBFiwkaPMAYUapWogsbVtVqMoGDx34BMGCwF6YDk3TDClzAkubMhCunNl29+sAAvD/xIty4ksBco001AjDAunXB3C6N/lSaubjx48iTi5ZIcaLjA6GVC5TdUoBJ0gAKaN9eYOZQ23yjR/8GUNWqQQLaDUR93jzBYZFJN748CX334J9mebJVC78hgYcziTeQZA9QNth+aKWE0FDEYQfZeQ41UFmBxBUEUwPqGRBAABspgJFGtGUkH0EbWeaTWQju58BOK5q1GQOMBWihAFXZ1NBGU2mFI2sHqOcbAE2dppQABzygQIUkAlDkAkICiUBmvGEmUG0CCGihbVYql6V0XHbp5ZdgHrTlQCOGOdcBiqXIAHRleomXgws20OZxViqQ05EHHZAahCQJsBdOLwb6Vk8prtXAkX3ZtRtGItm0XV631YVmTzbOZ5lNBjSq0ouFdXqYAs1B99CaDj0g40EEUqYkUiYJNEB66zn/RtFD8V0n04MWkkchejGB6txzJnGX3n8NuHRdAw1lCZOpJynAHEWf7rRZpwsokGlpQpU2X0wzfZTAq3nlVQCxA1kXlWIiCgVUQuOSNu5k6IWLV7sP9LaaRQFUqZmIYippJ1JkPVDtYe3p2YBU6hUgX5QDNZCUvgQ15K12GvmYFXvNKUAWXIUFGhhc0EWXbQFV+ilAkHy1xJu9zuKV42uhUVkQjkgRh5uOJ7O28mokOxmxQ5fpNuNkSr1WQFPx3kZAVKe9ttfKJivsbVCzlcpooIZ9WrBjTR2g8AAGfHQkWAgJiJ56z0nV3qfUFpYAaMaVXCVfnclrt7xjhqn33gQFAQQAOw==">
                    <span>SUITCASE</span>
                </div>
                <div class="anim-card" data-anim="20" onclick="selectAnim(this)">
                    <img class="anim-img" src="data:image/gif;base64,R0lGODdhMAAwAIMAAP////7+/v39/fz8/Pv7+/b29ufn58PDw5ycnHp6emFhYUBAQCIiIgwMDAMDAwAAACwAAAAAMAAwAEAI/wABCBxIsKDBgwgTKiwoQKCBBAseSJw4sYGCAwIHLEwYIICBBg0eJHCIQAEDiigfMEhwoECCBwYMPFAAQONCjQsaRFSQoKfPnQkUqEzAoIFPn0JXRsRoU6HGAw8cRFV58aEDAgBEQjXgsaRUB0JpNt2IUEDTAgYOqD3AdeBYsnDjyr0J4IBUkQILAMhpwEFOBQM0IpiJwG8BAQHgYh388kAAAh0hD3YsU0GABQwCIH4cACoCAQoaYE3MEUDf0DSxAiAwwDOAAjMBMFgwoMCAxwBeGigAdrXchgWEPjCqty/mqAYAIDiZ8wEC33OjS59OvXpB3ArNWi+I9UBElQj05v8MmfnASZAiGzacG2AAgZMSV7as+wCjyMF6TR8I6eDk87ccBcDAgAvUBIAArMn0HAF4WbQaa1jlJIBrqiWE00owFaDhho2hNVMBOW24YWUFTGZgQlgl4EBjBBRAQIsETEZAZQRg9uKNBHhGgG4H0gXVUgOlpWIDd1WkAALzudYAAz3C1Z4AEfmVnG0E6MSgSQXqBWIDBwjFFHWamRaaRDo5YCZ88T134nYHaRZYYIixKeecdNZpZ3QIshbYi7fdKVB7QSIQ1AIL8MSWW3Nq5BJ8K6loJqEULYBRR9V1F9JMydFHVEgYufRAoZ/qtd5vdRHpgJo1nRaASJjlqZxzYa3/SZZGL/WHwHc65aQWYSIRFd+SW3I1qkIpftqAAawNVIBOgn46mAL5OdSbAg4kR1pCDVW2InZP1TfjTO8tAOhqAQyWnE6yHoRVaDwWZO5rsWFmkEwjvTusQThNldK+/KYk1ZcoAmDSYBUO9O6qNMlrEF4mAkhQty8tmBF9lM0kIJPrdYsRZtc6tddsDJhFwHoErFgAVAqcRhBkA74K8EKILSDVAgbcWtGKL4FaFJkK5JRbfel6XKtzBOkoErXRDkDgcAYIcO9GHYEYH1MDNLakc9nmBBHRBc+1HgKXMoDAQ1NJKpxEkkK3nU01H0fRkhfZ9DSbj829Wp9+5q333nMGAQQAOw==">
                    <span>WORLD</span>
                </div>
            </div>
            <input type="hidden" id="anim" value="1">
        </div>
        
        <button class="btn" onclick="update()">SEND TO DEVICE</button>
        <div id="status" class="status">System Ready</div>
    </div>

    <script>
        function selectAnim(el) {
            document.querySelectorAll('.anim-card').forEach(c => c.classList.remove('active'));
            el.classList.add('active');
            document.getElementById('anim').value = el.dataset.anim;
        }
        function update() {
            const btn = document.querySelector('.btn');
            const msg = document.getElementById('msg').value;
            const anim = document.getElementById('anim').value;
            const autoplay = document.getElementById('autoplay').checked ? "1" : "0";
            const status = document.getElementById('status');
            
            btn.style.opacity = '0.7';
            btn.innerText = 'SENDING...';
            status.innerText = "Transmitting to ESP8266...";
            status.style.color = '#fff';
            
            fetch(`/update?msg=${encodeURIComponent(msg)}&anim=${anim}&autoplay=${autoplay}`)
                .then(r => r.text())
                .then(data => {
                    btn.style.opacity = '1';
                    btn.innerText = 'SEND TO DEVICE';
                    status.style.color = 'var(--primary)';
                    status.innerText = "Display Updated Successfully!";
                    setTimeout(() => {
                        status.innerText = "System Ready";
                        status.style.color = 'var(--primary)';
                    }, 3000);
                }).catch(err => {
                    btn.style.opacity = '1';
                    btn.innerText = 'SEND TO DEVICE';
                    status.style.color = '#ff6b6b';
                    status.innerText = "Connection Error!";
                });
        }
    </script>
</body>
</html>
)rawliteral";

// --- 텍스트 렌더링 헬퍼 함수 (우측 64x64 영역 줄바꿈, 수평/수직 중앙 정렬 처리) ---
void drawWrappedTextRight(String msg) {
  display.setTextSize(1);
  
  String lines[6];
  int lineCount = 0;
  String currentLine = "";
  int currentWidth = 0;
  
  for(unsigned i = 0; i < msg.length(); i++) {
    char c = msg[i];
    if (c == '\n') {
      lines[lineCount++] = currentLine;
      currentLine = "";
      currentWidth = 0;
      if (lineCount >= 6) break;
      continue;
    }
    
    if (currentWidth + 6 > 64) {
      lines[lineCount++] = currentLine;
      currentLine = String(c);
      currentWidth = 6;
      if (lineCount >= 6) break;
    } else {
      currentLine += c;
      currentWidth += 6;
    }
  }
  if (currentLine.length() > 0 && lineCount < 6) {
    lines[lineCount++] = currentLine;
  }
  
  int textHeight = lineCount * 10;
  int start_y = (64 - textHeight) / 2;
  if (start_y < 0) start_y = 0;
  
  for(int i = 0; i < lineCount; i++) {
    // Adafruit 기본 폰트는 폭 5px + 여백 1px = 6px 입니다. 
    // 문자열 길이에 비례해 직접 너비를 계산하여 예기치 않은 오류 방지
    int w = lines[i].length() * 6;
    
    int start_x = 64 + (64 - w) / 2;
    if (start_x < 64) start_x = 64; 
    
    display.setCursor(start_x, start_y + i * 10);
    display.print(lines[i]);
  }
}

// --- 애니메이션 함수들 ---

void drawStatic(String msg) {
  display.setTextSize(2);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (52 - h) / 2);
  display.print(msg);
}

void drawScroll(String msg) {
  display.setTextSize(2); // Reduced from 3 to 2 for better fit
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  
  if (millis() - lastUpdate > 30) {
    xPos -= 2;
    if (xPos < - (int)w) xPos = SCREEN_WIDTH;
    lastUpdate = millis();
  }
  // Vertical center within the top 52px (above the footer)
  display.setCursor(xPos, (52 - h) / 2);
  display.print(msg);
}

void drawBlink(String msg) {
  display.setTextSize(2);
  if (millis() - lastUpdate > 500) {
    blinkState = !blinkState;
    lastUpdate = millis();
  }
  if (blinkState) {
    int16_t x1, y1; uint16_t w, h;
    display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
    display.setCursor((SCREEN_WIDTH - w) / 2, (52 - h) / 2);
    display.print(msg);
  }
}

void drawTypewriter(String msg) {
  display.setTextSize(2);
  if (millis() - lastUpdate > 150) {
    charIndex++;
    if (charIndex > (int)msg.length()) {
       if (millis() - lastUpdate > 2000) charIndex = 0; // Wait at end
    } else {
       lastUpdate = millis();
    }
  }
  String sub = msg.substring(0, charIndex);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(sub, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (52 - h) / 2);
  display.print(sub);
  if (millis() % 1000 < 500) display.print("_");
}

void drawPulse(String msg) {
  if (millis() - lastUpdate > 40) {
    if (scaleUp) scale += 0.05; else scale -= 0.05;
    if (scale > 1.3) scaleUp = false;
    if (scale < 0.7) scaleUp = true;
    lastUpdate = millis();
  }
  display.setTextSize(2);
  int16_t x1, y1; uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  
  // Simple version: use size 3 when scale is high
  display.setTextSize((scale > 1.1) ? 3 : 2);
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, (52 - h) / 2);
  display.print(msg);
}

void drawIdeaAnimation(String msg) {
  static int currentIdeaFrame = 0;
  static unsigned long lastIdeaUpdate = 0;
  if (millis() - lastIdeaUpdate > 40) { // ~25 FPS
    currentIdeaFrame++;
    if (currentIdeaFrame >= IDEA_FRAMES) currentIdeaFrame = 0;
    lastIdeaUpdate = millis();
  }
  // 좌측 절반 애니메이션 렌더링
  display.drawBitmap(0, 0, idea_frames[currentIdeaFrame], IDEA_WIDTH, IDEA_HEIGHT, SSD1306_WHITE);
  // 우측 절반 텍스트
  drawWrappedTextRight(msg);
}

void drawLineChartAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= LINE_CHART_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, line_chart_frames[frameIdx], LINE_CHART_WIDTH, LINE_CHART_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawMeteorRainAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= METEOR_RAIN_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, meteor_rain_frames[frameIdx], METEOR_RAIN_WIDTH, METEOR_RAIN_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawSocialMediaAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= SOCIAL_MEDIA_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, social_media_frames[frameIdx], SOCIAL_MEDIA_WIDTH, SOCIAL_MEDIA_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawBarChartAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= BAR_CHART_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, bar_chart_frames[frameIdx], BAR_CHART_WIDTH, BAR_CHART_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawBookAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= BOOK_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, book_frames[frameIdx], BOOK_WIDTH, BOOK_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawCalendarAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= CALENDAR_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, calendar_frames[frameIdx], CALENDAR_WIDTH, CALENDAR_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawCloudNetworkAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= CLOUD_NETWORK_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, cloud_network_frames[frameIdx], CLOUD_NETWORK_WIDTH, CLOUD_NETWORK_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawFingerprintScanAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= FINGERPRINT_SCAN_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, fingerprint_scan_frames[frameIdx], FINGERPRINT_SCAN_WIDTH, FINGERPRINT_SCAN_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawHomeAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= HOME_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, home_frames[frameIdx], HOME_WIDTH, HOME_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawHotAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= HOT_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, hot_frames[frameIdx], HOT_WIDTH, HOT_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawInLoveAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= IN_LOVE_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, in_love_frames[frameIdx], IN_LOVE_WIDTH, IN_LOVE_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawLaptopAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= LAPTOP_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, laptop_frames[frameIdx], LAPTOP_WIDTH, LAPTOP_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawLocationAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= LOCATION_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, location_frames[frameIdx], LOCATION_WIDTH, LOCATION_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawMap1Animation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= MAP1_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, map1_frames[frameIdx], MAP1_WIDTH, MAP1_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawMap2Animation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= MAP2_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, map2_frames[frameIdx], MAP2_WIDTH, MAP2_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawMonitorAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= MONITOR_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, monitor_frames[frameIdx], MONITOR_WIDTH, MONITOR_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawPhotoCameraAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= PHOTO_CAMERA_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, photo_camera_frames[frameIdx], PHOTO_CAMERA_WIDTH, PHOTO_CAMERA_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawSuitcaseAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= SUITCASE_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, suitcase_frames[frameIdx], SUITCASE_WIDTH, SUITCASE_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

void drawWorldwideAnimation(String msg) {
  static int frameIdx = 0;
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 40) {
    frameIdx++;
    if (frameIdx >= WORLDWIDE_FRAMES) frameIdx = 0;
    lastUpdate = millis();
  }
  display.drawBitmap(0, 0, worldwide_frames[frameIdx], WORLDWIDE_WIDTH, WORLDWIDE_HEIGHT, SSD1306_WHITE);
  drawWrappedTextRight(msg);
}

// --- 서버 핸들러 ---

void handleRoot() {
  server.send(200, "text/html", HTML_CONTENT);
}

void handleUpdate() {
  if (server.hasArg("autoplay")) {
    autoplayMode = (server.arg("autoplay") == "1");
  }

  if (server.hasArg("anim")) {
    currentAnim = server.arg("anim").toInt();
    hasReceivedCommand = true;
    lastAnimSwitch = millis(); // Reset autoplay timer on manual selection
  }
  
  if (server.hasArg("msg")) {
    currentMsg = server.arg("msg");
    if (currentMsg.length() == 0 || currentMsg == " ") {
      // 입력 텍스트가 없으면, 선택한 애니메이션 타이틀을 기본 글자로 지정합니다.
      switch(currentAnim) {
        case 1: currentMsg = "IDEA"; break;
        case 2: currentMsg = "LINE\nCHART"; break;
        case 3: currentMsg = "METEOR\nRAIN"; break;
        case 4: currentMsg = "SOCIAL\nMEDIA"; break;
        case 5: currentMsg = "BAR\nCHART"; break;
        case 6: currentMsg = "BOOK"; break;
        case 7: currentMsg = "CALENDAR"; break;
        case 8: currentMsg = "CLOUD\nNET"; break;
        case 9: currentMsg = "FINGER\nPRINT"; break;
        case 10: currentMsg = "HOME"; break;
        case 11: currentMsg = "HOT"; break;
        case 12: currentMsg = "IN LOVE"; break;
        case 13: currentMsg = "LAPTOP"; break;
        case 14: currentMsg = "LOCATION"; break;
        case 15: currentMsg = "MAP 1"; break;
        case 16: currentMsg = "MAP 2"; break;
        case 17: currentMsg = "MONITOR"; break;
        case 18: currentMsg = "CAMERA"; break;
        case 19: currentMsg = "SUITCASE"; break;
        case 20: currentMsg = "WORLD"; break;
        default: currentMsg = "ANIM"; break;
      }
    }
  }
  
  xPos = SCREEN_WIDTH;
  charIndex = 0;
  scale = 1.0;
  server.send(200, "text/plain", "OK");
  Serial.print("New Message: "); Serial.println(currentMsg);
}

void setup() {
  Serial.begin(74880);
  Wire.begin(OLED_SDA, OLED_SCL);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // WiFi 연결 시도 (무한 루프)
  bool connected = false;
  int retryCount = 0;
  
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (!connected) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("NETWORK CONNECTING");
    display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
    
    display.setCursor(0, 20);
    display.print("SSID: "); display.println(WIFI_SSID);
    display.print("Retry: "); display.println(retryCount);
    
    display.setCursor(0, 45);
    for(int i=0; i<(retryCount % 16); i++) display.print(".");
    display.display();

    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
    } else {
      delay(1000);
      retryCount++;
      if (retryCount % 30 == 0) {
        WiFi.disconnect();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      }
    }
  }

  // 연결 성공 시 IP 표시
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SYSTEM ONLINE");
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setCursor(0, 25);
  display.println("IP ADDRESS:");
  display.setTextSize(2);
  display.println(WiFi.localIP().toString());
  display.display();
  
  Serial.println("\nWiFi Connected");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  MDNS.begin("billboard");
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();
  
  delay(3000); 
}

// Function renaming to avoid naming collision in loop
void drawBlinkingText(String msg) {
    drawBlink(msg);
}

void drawIPScreen() {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SYSTEM ONLINE");
  display.drawFastHLine(0, 12, 128, SSD1306_WHITE);
  display.setCursor(0, 25);
  display.println("IP ADDRESS:");
  display.setTextSize(2);
  display.println(WiFi.localIP().toString());
}

void loop() {
  server.handleClient();
  MDNS.update();

  // WiFi 연결 유지 확인 (5초 이상 끊겼을 때만 재부팅)
  static unsigned long wifiLostTime = 0;
  if (WiFi.status() != WL_CONNECTED) {
    if (wifiLostTime == 0) wifiLostTime = millis();
    if (millis() - wifiLostTime > 5000) {
      display.clearDisplay();
      display.setCursor(0, 20);
      display.println("RECONNECTING...");
      display.display();
      delay(1000);
      ESP.restart();
    }
  } else {
    wifiLostTime = 0;
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (!hasReceivedCommand) {
    // 1. 초기 IP 화면 렌더링
    drawIPScreen();
  } else {
    // Autoplay 처리
    if (autoplayMode) {
      if (millis() - lastAnimSwitch > 3000) {
        currentAnim++;
        if (currentAnim > 20) currentAnim = 1;
        
        // Update default message to match the new animation
        switch(currentAnim) {
          case 1: currentMsg = "IDEA"; break;
          case 2: currentMsg = "LINE\nCHART"; break;
          case 3: currentMsg = "METEOR\nRAIN"; break;
          case 4: currentMsg = "SOCIAL\nMEDIA"; break;
          case 5: currentMsg = "BAR\nCHART"; break;
          case 6: currentMsg = "BOOK"; break;
          case 7: currentMsg = "CALENDAR"; break;
          case 8: currentMsg = "CLOUD\nNET"; break;
          case 9: currentMsg = "FINGER\nPRINT"; break;
          case 10: currentMsg = "HOME"; break;
          case 11: currentMsg = "HOT"; break;
          case 12: currentMsg = "IN LOVE"; break;
          case 13: currentMsg = "LAPTOP"; break;
          case 14: currentMsg = "LOCATION"; break;
          case 15: currentMsg = "MAP 1"; break;
          case 16: currentMsg = "MAP 2"; break;
          case 17: currentMsg = "MONITOR"; break;
          case 18: currentMsg = "CAMERA"; break;
          case 19: currentMsg = "SUITCASE"; break;
          case 20: currentMsg = "WORLD"; break;
          default: currentMsg = "ANIM"; break;
        }

        lastAnimSwitch = millis();
      }
    }

    // 2. 전광판 애니메이션 렌더링 (하단바 없음)
    switch (currentAnim) {
      case 1: drawIdeaAnimation(currentMsg); break;
      case 2: drawLineChartAnimation(currentMsg); break;
      case 3: drawMeteorRainAnimation(currentMsg); break;
      case 4: drawSocialMediaAnimation(currentMsg); break;
      case 5: drawBarChartAnimation(currentMsg); break;
      case 6: drawBookAnimation(currentMsg); break;
      case 7: drawCalendarAnimation(currentMsg); break;
      case 8: drawCloudNetworkAnimation(currentMsg); break;
      case 9: drawFingerprintScanAnimation(currentMsg); break;
      case 10: drawHomeAnimation(currentMsg); break;
      case 11: drawHotAnimation(currentMsg); break;
      case 12: drawInLoveAnimation(currentMsg); break;
      case 13: drawLaptopAnimation(currentMsg); break;
      case 14: drawLocationAnimation(currentMsg); break;
      case 15: drawMap1Animation(currentMsg); break;
      case 16: drawMap2Animation(currentMsg); break;
      case 17: drawMonitorAnimation(currentMsg); break;
      case 18: drawPhotoCameraAnimation(currentMsg); break;
      case 19: drawSuitcaseAnimation(currentMsg); break;
      case 20: drawWorldwideAnimation(currentMsg); break;
      default: drawIdeaAnimation(currentMsg); break;
    }
  }

  // 루프 생존 표시기 (깜빡이는 점 - 최하단 구석, 방해되지 않게)
  if (millis() - statusMillis > 500) {
    statusDot = !statusDot;
    statusMillis = millis();
  }
  if (statusDot) display.drawPixel(127, 63, SSD1306_WHITE);

  display.display();
  delay(20);
}
