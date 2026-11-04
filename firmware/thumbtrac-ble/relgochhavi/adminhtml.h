#ifndef RELGO_ADMIN_HTML_H
#define RELGO_ADMIN_HTML_H

const char logentries_html[] = R"logpage(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Log Entries</title>
</head>
<body>
<h2>Download Log entries as CSV</h2>
<p>Click the link below to download the log entries as a CSV file:</p>
)logpage";

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
    <title>:: Relgo Thumb Trac ::</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style type="text/css">
        html {
            font-family: Arial, Helvetica, sans-serif;
            text-align: center;
        }
        body {
            margin: 0;
        }
        .topnav {
            overflow: hidden;
            background-color: #FF7F27;
        }
            .topnav h1 {
                font-size: 36px;
                color: #000;
                font-weight: 400;
                margin: 10px 0px;
            }
            .topnav h3 {
                font-size: 20px;
                color: #337ab7;
                margin: 0px 0px 15px 0px;
                font-weight: normal;
            }
        .content {
            padding: 30px;
            max-width: 800px;
            margin: 0 auto;
        }
        .card {
            background-color: #F8F7F9;
            /*box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);*/
            padding-top: 20px;
            padding-bottom: 20px;
            border-radius: 5px;
            border: solid 1px #d5d5d5;
            margin-left:200px;
            text-align:left;
            padding-left:50px;
        }
        .card h5 {
            font-size: 17px;
            padding: 0px;
            margin: 10px 0px;
            font-weight: normal;
        }
        input[type=text] {
            padding: 6px 12px;
            font-size: 14px;
            color: #555;
            background-color: #fff;
            background-image: none;
            border: 1px solid #ccc;
            border-radius: 4px;
            width: 75%;
        }
        .state {
            font-size: 1.5rem;
            color: #8c8c8c;
            font-weight: bold;
        }
    </style>
</head>

<body>
    <div class="home">
        <div class="topnav">
            <h1>Relgo Thumb Trac Administration</h1>
            <h3>Press command key for using your device.</h3>
        </div>
        <div class="content">
            <div class="card">
                <h5><strong>c</strong> : Configure Gateway Device  </h5>  
                <h5><strong>f</strong> : Register Fingerprint Device  </h5>  
                <h5><strong>n</strong> : Register NFC Device  </h5>  
                <h5><strong>l</strong> : Register Lock Device  </h5>  
                <h5><strong>m</strong> : Register Camera Device  </h5>
                <h5><strong>e</strong> : Enroll User (For No Service only) </h5>
                <h5><strong>u</strong> : Delete User (For No Service only) </h5>
                <h5><strong>i</strong> : Get Device Information  </h5>
                <h5><strong>t</strong> : Get Time log entries (For No Service only)  </h5>
                <h5><strong>d</strong> : Delete log entries   </h5><br/>
                <h5><strong>w</strong> : Wipe Device   </h5><br/>
                <label for="fname">Input :</label>
                <input type="text" id="fname" name="fname" onkeyup="commandevent()" autocomplete="off" placeholder="Enter Input Value"><br />
                <input type="text" id="fval" name="fval" onkeyup="commandevent()" autocomplete="off" placeholder="User# for Delete (u) command"><br />
                (Press Enter)
            </div>
        </div>
    </div>
    <script>
        function commandevent() {
            console.log("commandevent");
            // Number 13 is the "Enter" key on the keyboard
            if (event.keyCode == 13) {
                // Cancel the default action, if needed
                event.preventDefault();
                new_value = document.getElementById("fname").value;
                switch (new_value) {
                    case 'c':
                        window.location.assign("./configurePage");
                        break;
                    case 'l':
                        window.location.assign("./registerService?DoorLock");
                        break;
                    case 'n':
                        window.location.assign("./registerNFCService");
                        break;
                    case 'm':
                        window.location.assign("./registerCameraService");
                        break;
                    case 'f':
                        window.location.assign("./deviceMode?register");
                        break;
                    case 'e':
                        window.location.assign("./enrollUser");
                        break;
                    case 'u':
                        userid = document.getElementById("fval").value;
                        window.location.assign("./deleteUser?employeeCode="+userid);
                        break;
                    case 'i':
                        window.location.assign("./deviceMode?info");
                        break;
                    case 't':
                        window.location.assign("./deviceMode?logentries");
                        break;
                    case 'd':
                        window.location.assign("./resetDevice?logentries");
                        break;
                    case 'w':
                        window.location.assign("./resetDevice?wipe&templates&logentries");
                        break;
                    default:
                        window.location.assign("./deviceMode");
                        break;
                }
                document.getElementById("fname").value = "";
            }
        }
    </script>
</body>
</html>
)rawliteral";


const char config_html[] PROGMEM = R"configpage(
<!DOCTYPE HTML>
<html>
<head>
    <title>:: Relgo Thumb Trac ::</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
        html {
            font-family: Arial, Helvetica, sans-serif;
            text-align: left;
            margin: 0px auto;
        }

        body {
            margin: 0;
        }

        .topnav {
            overflow: hidden;
            background-color: #FF7F27;
            text-align: center;
        }

            .topnav h1 {
                font-size: 36px;
                color: #000;
                font-weight: 400;
                margin: 10px 0px;
            }

            .topnav h3 {
                font-size: 20px;
                color: #337ab7;
                margin: 0px 0px 15px 0px;
                font-weight: normal;
            }

        .content {
            padding: 30px;
            max-width: 800px;
            margin: 0 auto;
        }

        .card {
            background-color: #F8F7F9;
            box-shadow: 2px 2px 12px 1px rgba(140,140,140,.5);
            padding-top: 20px;
            padding-bottom: 20px;
        }

        input[type=text] {
            padding: 6px 12px;
            font-size: 14px;
            color: #555;
            background-color: #fff;
            background-image: none;
            border: 1px solid #ccc;
            border-radius: 4px;
            width: 300px;
        }

        .button {
            background-color: #337ab7;
            color: #fff;
            padding: 7px 15px 9px 15px;
            cursor: pointer;
            border: 0px;
            letter-spacing: 1.2px;
            margin-left:200px;
        }

        .state {
            font-size: 1.5rem;
            color: #8c8c8c;
            font-weight: bold;
        }
    </style>
</head>
<body>
    <div class="home">
        <div class="topnav">
            <h1>Configure Device</h1>
            <h3>Configure your device</h3>
        </div>
        <div class="content">
            <div class="card">
                <label for="devicename" style="display: inline-block; width: 200px; text-align: right;">Device Name </label>
                <input type="text" id="devicename" name="devicename" autocomplete="off" placeholder="Lobby"><br>
                <br>
                <label for="ssid" style="display: inline-block; width: 200px; text-align: right;">Wifi SSID </label>
                <input type="text" id="ssid" name="ssid" autocomplete="off" placeholder="Your Wifi User"><br>
                <br>
                <label for="password" style="display: inline-block; width: 200px; text-align: right;">Wifi Password </label>
                <input type="password" id="password" name="password" autocomplete="off" placeholder="Your Wifi Password"><br>
                <br>
                <label for="location" style="display: inline-block; width: 200px; text-align: right;">Location </label>
                <input type="text" id="location" name="location" autocomplete="off" width="200px" placeholder="Lobby(no-spaces)"><br>
                <br>
                <label for="timezone" style="display: inline-block; width: 200px; text-align: right;">Device Timezone:</label>
                <select id="timezoneselect" name="timezone" onchange='checkvalue(this.value)'>
                    <option value="PST/PDT" selected>PST/PDT</option>
                    <option value="EST/EDT">EST/EDT</option>
                    <option value="CST/CDT">CST/CDT</option>
                    <option value="MST/MDT">MST/CDT</option>
                    <option value="IST">IST</option>
                    <option value="OTH">OTH</option>
                </select>
                <input type="text" id="timezone" name="timezone" style="display: none; margin-left: 275px; margin-top: 15px;" autocomplete="off" placeholder="gmt offset in hours (5.5 or -8) if OTH is selected "><br>
                <br />
                <label for="servicetype" style="display: inline-block; width: 200px; text-align: right;">Service Type</label>
                <select id="servicetype" name="servicetype" onchange='servicelocation(this.value)'>
                    <option value="SingleLocation">No Relgo Service</option>
                    <option value="MultiLocation" selected>With Relgo Service</option>
                </select><br />
                <br />
                <label for="account" style="display: inline-block; width: 200px; text-align: right;">Service Account </label>
                <input type="text" id="account" name="account" autocomplete="off" placeholder="Service Admin Account works with  multi locations"><br>
                <br>
                <div id="div_multilocations" style="display: inline; padding-top: 15px;">
                    <label for="serviceurl" style="display: inline-block; width: 200px; text-align: right;">Service Url </label>
                    <input type="text" id="serviceurl" name="serviceurl" autocomplete="off" placeholder="Service Url works with  multi locations"><br>
                    <br>
                </div>
                <label for="license" style="display: inline-block; width: 200px; text-align: right;">License Type</label>
                <select id="license" name="license" onchange='setuplicense(this.value)'>
                    <option value="Trial">Trial</option>
                    <option value="Licensed" selected>Licensed</option>
                </select><br />
                <br/>
                <button class="button" onclick="configurecommand()">Configure</button>
            </div>
        </div>
    </div>
    <script>
        function configurecommand() {
            var configcommand = "./configureDevice?";
            var name = document.getElementById('devicename').value;
            var ssid = document.getElementById('ssid').value;
            var password = document.getElementById('password').value;
            var location = document.getElementById('location').value;
            var tzselect = document.getElementById('timezoneselect').value;
            var timezone;
            if (tzselect != "OTH")
                timezone = document.getElementById('timezoneselect').value;
            else
                timezone = tzselect;
            var servicetype = document.getElementById('servicetype').value;
            var account = document.getElementById('account').value;
            var serviceurl = document.getElementById('serviceurl').value;
            var licensetype = document.getElementById('license').value;
            var configcommand = "./configureDevice?" + "name=" + name + "&ssid=" + ssid + "&password=" + password + "&location=" + location + "&timezone=" + timezone + "&servicetype=" + servicetype + "&admin=" + account + "&server=" + serviceurl + "&licensetype=" + licensetype;
            console.log(configcommand);
            window.location.assign(configcommand);
        }

        function checkvalue(val) {
            if (val === "OTH")
                document.getElementById('timezone').style.display = 'block';
            else
                document.getElementById('timezone').style.display = 'none';
        }

        function servicelocation(val) {
            if (val === "SingleLocation")
                document.getElementById('div_multilocations').style.display = 'none';
            else
                document.getElementById('div_multilocations').style.display = 'block';
        }

    </script>
</body>
</html>

)configpage";

#endif
