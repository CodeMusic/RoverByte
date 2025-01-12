# Automatic file, do not edit!

module_auto = None
board = None
import time
from . import BaseModuleAuto
from makeblock.protocols.PackData import HalocodePackData
from makeblock.modules.cyberpi import event

if module_auto:
    event.set_mdoule_obj(module_auto)

def autoconnect():
    global module_auto
    if module_auto is None:
        module_auto = BaseModuleAuto(board)
        event.set_mdoule_obj(module_auto)
        # blocking to wait cyberpi start  
        board.broadcast()
        # go into online mode
        board.call(HalocodePackData.broadcast())


def goto_offline_mode():
    autoconnect()
    board.call(HalocodePackData([0xf3, 0xf6, 0x03, 0x0, 0x0d, 0x0, 0x00, 0x0e, 0xf4]))

def goto_online_mode():
    autoconnect()
    board.call(HalocodePackData([0xf3, 0xf6, 0x03, 0x0, 0x0d, 0x0, 0x01, 0x0e, 0xf4]))
def set_recognition_url(server = 1, url = "http://msapi.passport3.makeblock.com/ms/bing_speech/interactive"): 
    autoconnect()
    return module_auto.common_request("be139ea20b00673e9e8dcb7bc1640df9", (server , url ))
def get_mac_address():
    autoconnect()
    return module_auto.common_request("79b023fb7e007b2291a54784c3ab045f", () ,30)
def get_battery():
    autoconnect()
    return module_auto.get_value("3988e0aab4777855379065fc8da2ba34", ())
def get_shield():
    autoconnect()
    return module_auto.get_value("86b1f52cbe43f4d01c9c89cda63beb3c", ())
def get_extra_battery():
    autoconnect()
    return module_auto.get_value("e8e71442c9cadf8ca7c89bea4867872c", ())
def get_language():
    autoconnect()
    return module_auto.get_value("36a9f3e3395c6747d5b0d3fdde62366d", ())
def restart():
    autoconnect()
    return module_auto.common_request("7801409efa2cbc182d240320f247cf63", () ,30)
def is_makex_mode(name = "auto"):  
    autoconnect()
    return module_auto.get_value("6df1c7d3fbdd4da98151197c4825cf2e", (name ))
def get_firmware_version():
    autoconnect()
    return module_auto.common_request("84130d80d8bd240c93e5809565d4fb93", () ,30)
def get_ble():
    autoconnect()
    return module_auto.common_request("3636689f7387305b262ef85f68736204", () ,30)
def get_name():
    autoconnect()
    return module_auto.common_request("974853ae311b0874fd141e4dbc7ea504", () ,30)
def set_name(name):
    autoconnect()
    return module_auto.common_request("3d43914582aed35b01093bddb465dc5a", (name) ,30)
def get_brightness():
    autoconnect()
    return module_auto.get_value("69d00a7b2ddc1d85e7ed380663f01ecf", ())
def get_bri():
    autoconnect()
    return module_auto.get_value("04eeffd3a88cf7eee2bf0cb141518cfd", ())
def get_loudness(mode = "maximum"):  
    autoconnect()
    return module_auto.get_value("2469bdefeb4529102292af589e8a2efc", (mode ))
def is_tiltback():
    autoconnect()
    return module_auto.get_value("4cf92569435ea2828f6e3b6304907430", ())
def is_tiltforward():
    autoconnect()
    return module_auto.get_value("aeefd935140b9ce16ede5da7b7ada9fa", ())
def is_tiltleft():
    autoconnect()
    return module_auto.get_value("a4cf641a79cf925a87725098646118f9", ())
def is_tiltright():
    autoconnect()
    return module_auto.get_value("fec52f5ad18094161cec1b417c7e10f5", ())
def is_faceup():
    autoconnect()
    return module_auto.get_value("43c455ca4f76e8a737618fbef054bdbe", ())
def is_facedown():
    autoconnect()
    return module_auto.get_value("c6830cf56b1a4f9aaee9ab53e89a923d", ())
def is_stand():
    autoconnect()
    return module_auto.get_value("c1fb2bd86ac930a853470c2e99c81ea2", ())
def is_handstand():
    autoconnect()
    return module_auto.get_value("c6d9801ca9be3f1032f2be41538fa658", ())
def is_shake():
    autoconnect()
    return module_auto.get_value("c33fcd8e57ae1e3b4c513f6f3386b4b6", ())
def is_waveup():
    autoconnect()
    return module_auto.get_value("e880c1ca448156bf3f371490414e4ee8", ())
def is_wavedown():
    autoconnect()
    return module_auto.get_value("aefc483bd0bcd4561ee6d6d2abface7c", ())
def is_waveleft():
    autoconnect()
    return module_auto.get_value("2dd513e5fbaf94cfe7cafdc64ebbeb74", ())
def is_waveright():
    autoconnect()
    return module_auto.get_value("231b8c48c8b30f532899415153ce868e", ())
def is_freefall():
    autoconnect()
    return module_auto.get_value("dfabbe0e342dffec53fedfff55bff3e1", ())
def is_clockwise():
    autoconnect()
    return module_auto.get_value("fe32241d6feac2a268f7eafa488761eb", ())
def is_anticlockwise():
    autoconnect()
    return module_auto.get_value("196145516d82a638b39657e5b657df7c", ())
def get_shakeval():
    autoconnect()
    return module_auto.get_value("8a5db13c5c596e0407aa4abbe4f1db48", ())
def get_wave_angle():
    autoconnect()
    return module_auto.get_value("1632d0eb591aed54c5762a5dcdab81ed", ())
def get_wave_speed():
    autoconnect()
    return module_auto.get_value("822687ed58d16f303cc79e0b03bb6ce4", ())
def get_roll():
    autoconnect()
    return module_auto.get_value("1ff145c62ee8412c628e0b0a16fd67fc", ())
def get_pitch():
    autoconnect()
    return module_auto.get_value("36119451128173ff12497681ec2502e4", ())
def get_yaw():
    autoconnect()
    return module_auto.get_value("6bc201e4c360195c6ef04a99c3adb982", ())
def reset_yaw():
    autoconnect()
    return module_auto.common_request("8f4ae00969ac36f47100a06eee0d5046", () ,30)
def get_acc(axis):
    autoconnect()
    return module_auto.get_value("c9f33e1c6b89d173924779a21a9b1019", (axis))
def get_gyro(axis):
    autoconnect()
    return module_auto.get_value("55ddb68e0c157494d0f7e9825815ed43", (axis))
def get_rotation(axis):
    autoconnect()
    return module_auto.get_value("57f3a0363bf3394221b09dd8c8667892", (axis))
def reset_rotation(axis= "all"):
    autoconnect()
    return module_auto.common_request("2e150de47aabc09be2cd468491e39a37", (axis) ,30)

class controller_c():
    def is_press(self, name):
        autoconnect()
        return module_auto.get_value("0f4008600f10c30d9500bf0896cb3945", ( name))
    def get_count(self, name):
        autoconnect()
        return module_auto.get_value("f57ae52200b5f46041c7804eec7e0423", ( name))
    def reset_count(self, name):
        autoconnect()
        return module_auto.common_request("87aca6f56a1f74eab8abb4c29616ad3c", ( name) ,30)
controller=controller_c()

class audio_c():
    def play(self, music_name):
        autoconnect()
        return module_auto.common_request("99350cd953df88e5f025bcef34113717", ( music_name) ,30)
    def play_until(self, music_name):
        autoconnect()
        return module_auto.common_request("0fd6e3524a5e7c70ccb4ae7194c3ebd4", ( music_name) ,30)
    def record(self):
        autoconnect()
        return module_auto.common_request("8bd7053a4dd190e81637918be72deb46", () ,30)
    def stop_record(self):
        autoconnect()
        return module_auto.common_request("e5e11eee2dfa73a8626ad875188ce3ca", () ,30)
    def play_record_until(self):
        autoconnect()
        return module_auto.common_request("7e7984879e15188bbc90283564d6bdf5", () ,30)
    def play_record(self):
        autoconnect()
        return module_auto.common_request("a8d3656533d5343390a4545985c40c5d", () ,30)
    def play_tone(self, freq, t = None):
        autoconnect()
        return module_auto.common_request("611cfa431119957e2345e9db178252c6", ( freq, t ) ,30)
    def play_drum(self, type, beat):
        autoconnect()
        return module_auto.common_request("db5f575f0e9b43a42505970665e7f263", ( type, beat) ,30)
    def play_music(self, note, beat, type = "piano"):
        autoconnect()
        return module_auto.common_request("3a5b01ef1cc7cf8d5eb05aa275ab0d3f", ( note, beat, type ) ,30)
    def play_note(self, note, beat):
        autoconnect()
        return module_auto.common_request("487b1f0a257ab4cad6edd09ea1df35a0", ( note, beat) ,30)
    def add_tempo(self, pct):
        autoconnect()
        return module_auto.common_request("7cf9bc83ad4148ac607d266d30a4d873", ( pct) ,30)
    def set_tempo(self, pct):
        autoconnect()
        return module_auto.common_request("97f80d0c8e36537051d6c9b21e8a7e56", ( pct) ,30)
    def get_tempo(self):
        autoconnect()
        return module_auto.common_request("0b4dbd2d04bdaac5cf2f578c6087d58d", () ,30)
    def add_vol(self, val):
        autoconnect()
        return module_auto.common_request("f4b0497ab889a85a865a7486dc9c3b20", ( val) ,30)
    def set_vol(self, val):
        autoconnect()
        return module_auto.common_request("c2e5e981c561ba61d2d757a61ff17baa", ( val) ,30)
    def get_vol(self):
        autoconnect()
        return module_auto.common_request("4dedceaadf514a624dea51612e1d3f31", () , 30)
    def stop(self):
        autoconnect()
        return module_auto.common_request("510b2145fdd0714503879cc98c435d81", () , 30, w_mode= 3)
audio=audio_c()

class display_c():
    def set_brush(self, r = 0, g = 0, b = 0):
        autoconnect()
        return module_auto.common_request("3b6fa373ef8ec0614b60949d7a6498eb", ( r , g , b ) , 30)
    def set_title_color(self, r = 0, g = 0, b = 0):
        autoconnect()
        return module_auto.common_request("f5d5d6501e3afd18d87e81b0ec5eadb4", ( r , g , b ) , 30)
    def rotate_to(self, angle):
        autoconnect()
        return module_auto.common_request("702530b11223d816f2a79852a5298b7c", ( angle) ,30)
    def clear(self):
        autoconnect()
        return module_auto.common_request("91aee200649793797d3c8194f3c9e751", () ,30)
    def off(self):
        autoconnect()
        return module_auto.common_request("064b32a98954fee5dba9c08ce8e9ad37", () , 30)
    def label(self, message, size, x = 0, y = 0, new_flag = False):
        autoconnect()
        return module_auto.common_request("2b099ea04f7109e654b4556d98c89caa", ( message, size, x , y , new_flag ) ,30)
    def show_label(self, message, size, x = 0, y = 0, index = None, new_flag = False):
        autoconnect()
        return module_auto.common_request("e3d7894d178c918ae0d4ec9dc34c4765", ( message, size, x , y , index , new_flag ) ,30)
display=display_c()

class console_c():
    def clear(self):
        autoconnect()
        return module_auto.common_request("dbeeb276871d7a91b2466c5fc9c02222", () ,30)
    def print(self, message):
        autoconnect()
        return module_auto.common_request("5d64cc86824e28e871fb85902181ff18", ( message) ,30)
    def println(self, message):
        autoconnect()
        return module_auto.common_request("778c0e54274e093926b2012fa3cc6ba4", ( message) ,30)
console=console_c()

class chart_c():
    def set_name(self, name):
        autoconnect()
        return module_auto.common_request("fb465e33a0d73e1ff32e945c30f12836", ( name) , 30)
    def clear(self):
        autoconnect()
        return module_auto.common_request("e2c4c0b767853bfb5254e88235ce635b", () , 30)
chart=chart_c()

class linechart_c():
    def add(self, data):
        autoconnect()
        return module_auto.common_request("9c7c0606827e5299122c6b71119a8bab", ( data) ,30)
    def set_step(self, step):
        autoconnect()
        return module_auto.common_request("53e2d964d0bb39fb905c5b6b63904c43", ( step) , 30)
linechart=linechart_c()

class barchart_c():
    def add(self, data):
        autoconnect()
        return module_auto.common_request("953af5bbd66750092d3a1406d53399f3", ( data) ,30)
barchart=barchart_c()

class table_c():
    def add(self, row, column, data):
        autoconnect()
        return module_auto.common_request("2516ef92512360a0c5a7cb54cfa30d7d", ( row, column, data) ,30)
table=table_c()

class led_c():
    def on(self, r = 0, g = 0, b = 0, id = "all"):
        autoconnect()
        return module_auto.common_request("5119428dbfeb589dac36fb628ca9248d", ( r , g , b , id ) , 30)
    def play(self, name = "rainbow"):
        autoconnect()
        return module_auto.common_request("0c6872183c51d59d859becb64667c349", ( name ) ,30)
    def show(self, color, offset = 0):
        autoconnect()
        return module_auto.common_request("c1de77fafa5b4a4e059515b451a5378d", ( color, offset ) , 30)
    def move(self, step = 1):
        autoconnect()
        return module_auto.common_request("3164e897ffb74c0994c83a84f4938989", ( step ) , 30)
    def off(self, id = "all"):
        autoconnect()
        return module_auto.common_request("883bd62fb06060130e7da7ef5eb29eb6", ( id ) , 30)
    def add_bri(self, brightness):
        autoconnect()
        return module_auto.common_request("12d6db993b5874c141e65e740383fd09", ( brightness) , 30)
    def set_bri(self, brightness):
        autoconnect()
        return module_auto.common_request("8fd93cf231967274847b712d132b2647", ( brightness) , 30)
    def get_bri(self):
        autoconnect()
        return module_auto.common_request("f32939dd291b3df36b000e28357ec849", () , 30)
led=led_c()

class wifi_c():
    def connect(self, ssid, password):
        autoconnect()
        return module_auto.common_request("bd1de88cc76a8dbff3a0adea09a34234", ( ssid, password) , 30)
    def is_connect(self):
        autoconnect()
        return module_auto.common_request("f0b19cff8669fd061bf3bf8d755a9eee", () , 30)
wifi=wifi_c()

class cloud_c():
    def setkey(self, key):
        autoconnect()
        return module_auto.common_request("8161e0b45af7e1d12e2bd714b8b3d47f", ( key) , 30)
    def weather(self, option, woe_id):
        autoconnect()
        return module_auto.common_request("434de984f1ad299d904e3deb2cb002e2", ( option, woe_id) , 30)
    def air(self, option, woe_id):
        autoconnect()
        return module_auto.common_request("402dae9adbacff802492236ba6c977c2", ( option, woe_id) , 30)
    def time(self, option, location):
        autoconnect()
        return module_auto.common_request("70279bf87312616ec7e74ab28530fb32", ( option, location) , 30)
    def listen(self, language, t = 3):
        autoconnect()
        return module_auto.common_request("97adee4bd25bc6c77195451d5e7a42d6", ( language, t ) , 30)
    def listen_result(self):
        autoconnect()
        return module_auto.common_request("10b07f3c97a2833abaeea5b42aa892b7", () , 30)
    def tts(self, language, message):
        autoconnect()
        return module_auto.common_request("093cf48163eb9ae0997ea9f6a521c2f2", ( language, message) , 30)
    def translate(self, language, message):
        autoconnect()
        return module_auto.common_request("716553fa3b4c7469133cf3d465536cd5", ( language, message) , 30)
    def translate_set_url(self, url):
        autoconnect()
        return module_auto.common_request("da4b3d9a6aa1383c067ccfa275fec980", ( url) , 30)
    def tts_set_url(self, url):
        autoconnect()
        return module_auto.common_request("87a6b1c926b2c688906aa577924d8c1d", ( url) , 30)
    def recognition_set_url(self, url):
        autoconnect()
        return module_auto.common_request("db2fc952af6866e32d92e5f2244ff3b3", ( url) , 30)
cloud=cloud_c()

class timer_c():
    def get(self):
        autoconnect()
        return module_auto.get_value("5535075a55d873b7450bad031ba3ba72", ())
    def reset(self):
        autoconnect()
        return module_auto.common_request("e5359d1a429dbef33644fa7d5e450ab2", () , 30)
timer=timer_c()
def broadcast(message):
    autoconnect()
    return module_auto.common_request("973eab065d5f700522b11bff0540f19a", (message) , 30)
def broadcast_and_wait(message):
    autoconnect()
    return module_auto.common_request("fe662895f3ea37bdb913466a5b6ba520", (message) , 30)

class wifi_broadcast_c():
    def set(self, message, value = 0):
        autoconnect()
        return module_auto.common_request("e0b9fb05284e48b4d7a4a97c25eca5de", ( message, value ) ,30)
    def get(self, message):
        autoconnect()
        return module_auto.common_request("f125d69e4401eb06e48ccd6bb3d6c895", ( message) ,30)
wifi_broadcast=wifi_broadcast_c()

class mesh_c():
    def set(self, name):
        autoconnect()
        return module_auto.common_request("9423af40e94c7885bda8a15f431b191f", ( name) ,30)
    def join(self, name):
        autoconnect()
        return module_auto.common_request("63e9d3437fc269eb1dff822e1efab4ad", ( name) ,30)
    def get_info_once(self, message):
        autoconnect()
        return module_auto.common_request("7fa7ab3b41e128adefb209e63132a738", ( message))
mesh=mesh_c()

class mesh_broadcast_c():
    def set(self, message, value = 0):
        autoconnect()
        return module_auto.common_request("66c14b1d0cf03d5f1d5c1ced96626c3c", ( message, value ) ,30)
    def get(self, message):
        autoconnect()
        return module_auto.common_request("12e8237aaf419f3183026f372c2b80d1", ( message) ,30)
mesh_broadcast=mesh_broadcast_c()

class upload_broadcast_c():
    def set(self, message, value = 0):
        autoconnect()
        return module_auto.common_request("71de891af2a5dcbb9a8f4ed958060137", ( message, value ) ,30)
    def get(self, message):
        autoconnect()
        return module_auto.common_request("4b09d734021e25bcf990c7896f0ed816", ( message) ,30)
upload_broadcast=upload_broadcast_c()

class cloud_broadcast_c():
    def set(self, message, value = 0):
        autoconnect()
        return module_auto.common_request("91f7a4c19cdcf0592e368049f5cb04ea", ( message, value ) ,30)
    def get(self, message):
        autoconnect()
        return module_auto.common_request("d51ad9449c1a722b7df593a3ea603ee0", ( message) ,30)
    def get_info_online(self, message):
        autoconnect()
        return module_auto.get_value("db996e364162c972990c3a7497e74d38", ( message))
cloud_broadcast=cloud_broadcast_c()

class pocket_c():
    def motor_add(self, power, port):
        autoconnect()
        return module_auto.common_request("f14cf3698d0a66c7e7dc137979208559", ( power, port) , 30)
    def motor_set(self, power, port):
        autoconnect()
        return module_auto.common_request("0bbfb936f93f376709b85ce198b36ebe", ( power, port) , 30)
    def motor_get(self, port):
        autoconnect()
        return module_auto.common_request("d9f5ba4f16b1e646a7ee49bc4309984b", ( port) , 30)
    def motor_drive(self, power1, power2):
        autoconnect()
        return module_auto.common_request("9709a31105debd6284eaafa028e08851", ( power1, power2) , 30)
    def motor_stop(self, port):
        autoconnect()
        return module_auto.common_request("4841509eb9dd1beffdf13d245f81c030", ( port) , 30)
    def servo_add(self, angle, port):
        autoconnect()
        return module_auto.common_request("8f7607cc42694039b9c05a92ac38d096", ( angle, port) , 30)
    def servo_set(self, angle, port):
        autoconnect()
        return module_auto.common_request("776ebb4dd454816c9109e050c4223ec4", ( angle, port) , 30)
    def servo_get(self, port):
        autoconnect()
        return module_auto.common_request("be36631cd53eaf910c4e9070ac09ec27", ( port) , 30)
    def servo_release(self, port):
        autoconnect()
        return module_auto.common_request("42fb7c303ee2d34c2951128d98955f37", ( port) , 30)
    def servo_drive(self, angle1, angle2):
        autoconnect()
        return module_auto.common_request("11e6a43ca3a2eaf8ba8925cbf4a0b9a1", ( angle1, angle2) , 30)
    def led_on(self, r, g = 0, b = 0, id = "all", port = "all"):
        autoconnect()
        return module_auto.common_request("dec8a1a20537cde484678e9cfeed4ff5", ( r, g , b , id , port ) , 30)
    def led_show(self, color, port):
        autoconnect()
        return module_auto.common_request("d963c1df3f3bf425ce3a54941ec6eb1b", ( color, port) , 30)
    def led_move(self, step, cycle, port):
        autoconnect()
        return module_auto.common_request("8e5a4f975d3655c574cd88b74a775d43", ( step, cycle, port) , 30)
    def led_off(self, id = 'all', port = 'S1'):
        autoconnect()
        return module_auto.common_request("a2155c2883e4ade480316b26a29d3841", ( id , port ) , 30)
    def led_add_bri(self, brightness, port):
        autoconnect()
        return module_auto.common_request("34c16d2f099b94f7399869a4baa24ee7", ( brightness, port) , 30)
    def led_set_bri(self, brightness, port):
        autoconnect()
        return module_auto.common_request("201d614cd682f60391f4af94c8784910", ( brightness, port) , 30)
    def led_get_bri(self, port):
        autoconnect()
        return module_auto.common_request("828e20bbbe7048388e0f991ea7d75743", ( port) ,30)
    def write_digital(self, val, port):
        autoconnect()
        return module_auto.common_request("d798a5bda868c3cf30fe9015dc758384", ( val, port) , 30)
    def read_digital(self, port):
        autoconnect()
        return module_auto.common_request("604ffb911872fb6f62907f266b35c31c", ( port) ,30)
    def set_pwm(self, duty, frequency, port):
        autoconnect()
        return module_auto.common_request("718a577e169f580a16dd6ac78a4a270a", ( duty, frequency, port) , 30)
    def read_analog(self, port):
        autoconnect()
        return module_auto.common_request("cc75739dc5143b3863dcfa023a33fc06", ( port) ,30)
pocket=pocket_c()

class button_c():
    def is_press(self, index = 1):
        autoconnect()
        return module_auto.get_value("2fd7fdd9cc364e93839af5093eccada0", ( index ))
    def get_count(self, index = 1):
        autoconnect()
        return module_auto.get_value("6036b0c4c2a8de19dcfe66bc771b708f", ( index ))
    def reset_count(self, index = 1):
        autoconnect()
        return module_auto.common_request("4d3d6225ee7b02a374f7d6d9902d59e5", ( index ) ,30)
button=button_c()

class angle_sensor_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("466a0a7e36e2f0833c8f9712dc1347da", ( index ))
    def reset(self, index = 1):
        autoconnect()
        return module_auto.common_request("d357c560efdea10412a3fd77b6e7407d", ( index ) ,30)
    def get_speed(self, index = 1):
        autoconnect()
        return module_auto.get_value("653c264f992c745f19595f198e366064", ( index ))
    def is_clockwise(self, index = 1):
        autoconnect()
        return module_auto.get_value("74beee720507e4f16fcf069e8258ff1b", ( index ))
    def is_anticlockwise(self, index = 1):
        autoconnect()
        return module_auto.get_value("767a7284b254922aac50bb54b3304af9", ( index ))
angle_sensor=angle_sensor_c()

class slider_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("4218158c5f90b52093f9b633567028cb", ( index ))
slider=slider_c()

class joystick_c():
    def get_x(self, index = 1):
        autoconnect()
        return module_auto.get_value("329b0ec2001972b145f60dac9243800c", ( index ))
    def get_y(self, index = 1):
        autoconnect()
        return module_auto.get_value("3c6aa9e81a72607205a5fe22c0a7b34a", ( index ))
    def is_up(self, index = 1):
        autoconnect()
        return module_auto.get_value("d26cf2d9b787ce3f50582ad3e9683c91", ( index ))
    def is_down(self, index = 1):
        autoconnect()
        return module_auto.get_value("6e41766690b2a7a76a45a5fc83075707", ( index ))
    def is_left(self, index = 1):
        autoconnect()
        return module_auto.get_value("702ed21aa14e43084db76801d297c1e9", ( index ))
    def is_right(self, index = 1):
        autoconnect()
        return module_auto.get_value("7c08a552c6941671baf17b4760ba3b9a", ( index ))
joystick=joystick_c()

class multi_touch_c():
    def is_touch(self, ch = 'any', index = 1):
        autoconnect()
        return module_auto.get_value("7dc6c2793ae0b47221c01421cc493a90", ( ch , index ))
    def set(self, level = "middle", index = 1):
        autoconnect()
        return module_auto.common_request("7996e1354d54932fc066fbbfd4cd58c5", ( level , index ) ,30)
    def reset(self, level = "middle", index = 1):
        autoconnect()
        return module_auto.common_request("bc19c76773a823bcd72b6a3cc3bf64d2", ( level , index ) ,30)
multi_touch=multi_touch_c()

class light_sensor_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("61a4e7daeb90a389e55f49cd52da3087", ( index ))
light_sensor=light_sensor_c()

class dual_rgb_sensor_c():
    def get_red(self, ch = 1, index = 1):
        autoconnect()
        return module_auto.get_value("0a4a48ee6c3a39125e3bcc4d22b61bd9", ( ch , index ))
    def get_green(self, ch = 1, index = 1):
        autoconnect()
        return module_auto.get_value("ec148aee5faa180529bdd9ae1b6fb769", ( ch , index ))
    def get_blue(self, ch = 1, index = 1):
        autoconnect()
        return module_auto.get_value("89c4a7530f4dd5c02d45df34c9c1e99d", ( ch , index ))
    def is_color(self, color = 'white', ch = 1, index = 1):
        autoconnect()
        return module_auto.get_value("11fc67e6b3a0491c133e63ee203d67b0", ( color , ch , index ))
    def get_light(self, ch = 1, index = 1):
        autoconnect()
        return module_auto.get_value("47599c2d9bc858c9da8eb95b8082a12b", ( ch , index ))
    def get_gray_level(self, ch = 1, index = 1):
        autoconnect()
        return module_auto.get_value("21529a6b724230b961b667fd22e4b0d9", ( ch , index ))
    def set_led(self, color = 'white', index = 1):
        autoconnect()
        return module_auto.common_request("94ecd63bfd0ded6367f78cac36147008", ( color , index ) , 30)
    def off_led(self, index = 1):
        autoconnect()
        return module_auto.common_request("8acfd8aef4f03c2c405cff8d10d850a0", ( index ) , 30)
dual_rgb_sensor=dual_rgb_sensor_c()

class sound_sensor_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("4803135ff11d9ddf810a4558215d3c05", ( index ))
sound_sensor=sound_sensor_c()

class pir_c():
    def is_detect(self, index = 1):
        autoconnect()
        return module_auto.get_value("33adf300e6cf16c79c42d51ef4138434", ( index ))
    def get_count(self, index = 1):
        autoconnect()
        return module_auto.get_value("637f77cf83a15fb73181d8248d2209f8", ( index ))
    def reset_count(self, index = 1):
        autoconnect()
        return module_auto.common_request("8ad1d67e0b59f87a4deb3750b6b1898c", ( index ) ,30)
pir=pir_c()

class ultrasonic_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("53b508a9d07ca4ea6fdc127746f26353", ( index ))
ultrasonic=ultrasonic_c()

class ranging_sensor_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("336a80558295423ce35698210a1314c9", ( index ))
ranging_sensor=ranging_sensor_c()

class motion_sensor_c():
    def is_tiltleft(self, index = 1):
        autoconnect()
        return module_auto.get_value("cb3a28f5868da4a112215c0c2d0cd80e", ( index ))
    def is_tiltright(self, index = 1):
        autoconnect()
        return module_auto.get_value("1b50341ddeac7bdfe6a70b6d0326e703", ( index ))
    def is_tiltup(self, index = 1):
        autoconnect()
        return module_auto.get_value("07aba7578dc5725e307cc0d6473f1519", ( index ))
    def is_tiltdown(self, index = 1):
        autoconnect()
        return module_auto.get_value("2ae39725fb057646d13496e3451541c9", ( index ))
    def is_faceup(self, index = 1):
        autoconnect()
        return module_auto.get_value("9915f25af1ad344d6acb3b4f9a669c3b", ( index ))
    def is_facedown(self, index = 1):
        autoconnect()
        return module_auto.get_value("236094366aec9449878d5242bc4c6d64", ( index ))
    def is_shake(self, index = 1):
        autoconnect()
        return module_auto.get_value("26c0ae50fe1701d6489a197a71e3613f", ( index ))
    def get_shakeval(self, index = 1):
        autoconnect()
        return module_auto.get_value("5cc3f3c28bd844b90f44f194ad7650cf", ( index ))
    def get_accel(self, axis, index = 1):
        autoconnect()
        return module_auto.get_value("7a29c6c9ccb4772f95539b22d1e4889e", ( axis, index ))
    def get_gyro(self, index = 1):
        autoconnect()
        return module_auto.get_value("a0a6c73408a7358641fbc419b703e51d", ( index ))
    def get_roll(self, index = 1):
        autoconnect()
        return module_auto.get_value("9c1816917b4f5c2488fcc98898cf9800", ( index ))
    def get_pitch(self, index = 1):
        autoconnect()
        return module_auto.get_value("bf545bd2aead7ec5841b1ae355433fa1", ( index ))
    def get_yaw(self, index = 1):
        autoconnect()
        return module_auto.get_value("02ec9ecd2e720e53f308d71ad809ec04", ( index ))
    def get_rotation(self, index = 1):
        autoconnect()
        return module_auto.get_value("066e7834857a4070facfb0baf6b344da", ( index ))
    def reset_rotation(self, index = 1):
        autoconnect()
        return module_auto.common_request("8c5f098af82a67337f2aa2965872878a", ( index ) ,30)
motion_sensor=motion_sensor_c()

class soil_sensor_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("6729612451f44c7d9e968c5bb183380a", ( index ))
soil_sensor=soil_sensor_c()

class temp_sensor_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("8a7a5671b5bfbe8581afabbea4b28064", ( index ))
temp_sensor=temp_sensor_c()

class humiture_c():
    def get_humidity(self, index = 1):
        autoconnect()
        return module_auto.get_value("cb1d9f1546082a8fa7d5d3b7d39691dc", ( index ))
    def get_temp(self, index = 1):
        autoconnect()
        return module_auto.get_value("37663d09a8728687d7dae5938ec173fe", ( index ))
humiture=humiture_c()

class mq2_c():
    def is_detect(self, level = 'high',index = 1):
        autoconnect()
        return module_auto.get_value("c4ca0e449ec7efbbc2867ccb25972fdb", ( level ,index ))
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("8e5afd5fb157dd54f831e29126bd1dac", ( index ))
mq2=mq2_c()

class flame_sensor_c():
    def is_detect(self, index = 1):
        autoconnect()
        return module_auto.get_value("fdf535e5437a3f63b1c53497cb0bf6e6", ( index ))
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("3918994d8aa93f4b108b29914a5a9c2f", ( index ))
flame_sensor=flame_sensor_c()

class magnetic_sensor_c():
    def is_detect(self, index = 1):
        autoconnect()
        return module_auto.get_value("a1d01f6784d3fbdb25c9890c475fa1af", ( index ))
    def get_count(self, index = 1):
        autoconnect()
        return module_auto.get_value("a9be5788b430f72e8890e53076a45f87", ( index ))
    def reset_count(self, index = 1):
        autoconnect()
        return module_auto.common_request("514caf43affa7206b98961d67b39b2a5", ( index ) ,30)
magnetic_sensor=magnetic_sensor_c()

class led_matrix_c():
    def show(self, image = "hi", index = 1):
        autoconnect()
        return module_auto.common_request("0a36b1142cd9c57d431cd40fcdb52a63", ( image , index ) , 30)
    def show_at(self, image = "hi", x = 0, y = 0, index = 1):
        autoconnect()
        return module_auto.common_request("0a4f300dd408e4907a88dbc26bdf862a", ( image , x , y , index ) , 30)
    def print(self, message, index = 1):
        autoconnect()
        return module_auto.common_request("39aada5160f5d18f80c0a54d650f8c34", ( message, index ) , 30)
    def print_until_done(self, message, index = 1):
        autoconnect()
        return module_auto.common_request("4fc8f6ed7492088f3c3e370cdea18fee", ( message, index ) ,30)
    def print_at(self, message, x = 0, y = 0, index = 1):
        autoconnect()
        return module_auto.common_request("fe1474ef0739be5d937398d958f3dfd0", ( message, x , y , index ) , 30)
    def on(self, x = 0, y = 0, index = 1):
        autoconnect()
        return module_auto.common_request("9cbb10b41f3f5d048cf288a9ddcaf41f", ( x , y , index ) , 30)
    def off(self, x = 0, y = 0, index = 1):
        autoconnect()
        return module_auto.common_request("6e4d10fcd4896f23af930ec3c5da811e", ( x , y , index ) , 30)
    def toggle(self, x = 0, y = 0, index = 1):
        autoconnect()
        return module_auto.common_request("13be3cd3d99e375098a47eb4f27a641f", ( x , y , index ) , 30)
    def get(self, x = 0, y = 0, index = 1):
        autoconnect()
        return module_auto.common_request("fc7e4f7e6d6d38732780cf9cb92882e4", ( x , y , index ) , 30)
    def clear(self, index = 1):
        autoconnect()
        return module_auto.common_request("2266736814ffa6880ada32aadecba3f7", ( index ) , 30)
led_matrix=led_matrix_c()

class rgb_led_c():
    def on(self, r = 0, g = 0, b = 0, index = 1):
        autoconnect()
        return module_auto.common_request("c5b5ca3035fe0e8e928cd4f77abf3136", ( r , g , b , index ) , 30)
    def off(self, index = 1):
        autoconnect()
        return module_auto.common_request("dd0462307d855e7aad2b1c4d5cb92b4d", ( index ) , 30)
    def set_red(self, val, index = 1):
        autoconnect()
        return module_auto.common_request("73ebb52c77427ea8e302214bb9f03e7d", ( val, index ) , 30)
    def set_green(self, val, index = 1):
        autoconnect()
        return module_auto.common_request("3ed525ae3a3fb8c4c3d99ce38159ab12", ( val, index ) , 30)
    def set_blue(self, val, index = 1):
        autoconnect()
        return module_auto.common_request("e7d5243d81bb07178476fd5b138132ed", ( val, index ) , 30)
    def add_red(self, val, index = 1):
        autoconnect()
        return module_auto.common_request("32d74e7fc72a8b8d368c56544525b1f8", ( val, index ) , 30)
    def add_green(self, val, index = 1):
        autoconnect()
        return module_auto.common_request("8b37c95d064ca1de35a143bb1c7cf988", ( val, index ) , 30)
    def add_blue(self, val, index = 1):
        autoconnect()
        return module_auto.common_request("353325d9328194c8808750c8ebbe7574", ( val, index ) , 30)
    def get_red(self, index = 1):
        autoconnect()
        return module_auto.common_request("f5a3ca4d2df5fe90db4adce91b6aa20a", ( index ) , 30)
    def get_green(self, index = 1):
        autoconnect()
        return module_auto.common_request("771d65f5e53a13009bfc0d5d39b8e4e8", ( index ) ,30)
    def get_blue(self, index = 1):
        autoconnect()
        return module_auto.common_request("519688ecd9013344908dc499a3f3119d", ( index ) ,30)
rgb_led=rgb_led_c()

class led_driver_c():
    def on(self, r = 0, g = 0, b = 0, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("40fc880df668751be85b28ab7c866a26", ( r , g , b , id , index ) , 30)
    def off(self, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("d9fc3c5cc6dfccb58fa6df5aed1c4d96", ( id , index ) , 30)
    def set_red(self, val, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("a437fec37c65f9a8782cffce9db6bd1e", ( val, id , index ) , 30)
    def set_green(self, val, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("0fca0243087084fec4033bc78f502398", ( val, id , index ) , 30)
    def set_blue(self, val, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("33810988a6530c3d432ea8ad0cc96776", ( val, id , index ) , 30)
    def add_red(self, val, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("d9ae3f9c820e0775b45f62a3aca40bc2", ( val, id , index ) , 30)
    def add_green(self, val, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("62516697866c2a740bfdb32a810ba2f0", ( val, id , index ) , 30)
    def add_blue(self, val, id = "all", index = 1):
        autoconnect()
        return module_auto.common_request("2be3b01fe53c485e0d0eb80c8544561d", ( val, id , index ) , 30)
    def set_mode(self, mode = "steady", index = 1):
        autoconnect()
        return module_auto.common_request("679267a195b346e01bc94ac005e4e735", ( mode , index ) , 30)
    def show(self,color, index = 1):
        autoconnect()
        return module_auto.common_request("e820d887deeeb8b1b4cec6f9b1a22c77", (color, index ) , 30)
led_driver=led_driver_c()

class servo_driver_c():
    def set(self, angle, index = 1):
        autoconnect()
        return module_auto.common_request("599a9d0aaeae8af163b572258b842299", ( angle, index ) , 30)
    def add(self, angle, index = 1):
        autoconnect()
        return module_auto.common_request("24634711504cff12ed674d3817bf2669", ( angle, index ) , 30)
    def get(self, index = 1):
        autoconnect()
        return module_auto.common_request("d6cd64b1fd1733f66a17240097819f38", ( index ) ,30)
    def get_load(self, index = 1):
        autoconnect()
        return module_auto.common_request("a70fe6d7eed8e3fb23f624ede018c1f2", ( index ) ,30)
    def release(self, index = 1):
        autoconnect()
        return module_auto.common_request("2b0020324c91fa7e5dcb1a738190f293", ( index ) , 30)
servo_driver=servo_driver_c()

class motor_driver_c():
    def set(self, power, index = 1):
        autoconnect()
        return module_auto.common_request("5558035e8749d8bcc350a1128787b69e", ( power, index ) , 30)
    def add(self, power, index = 1):
        autoconnect()
        return module_auto.common_request("5ca600766d7058cee7739f7a17338513", ( power, index ) , 30)
    def get(self, index = 1):
        autoconnect()
        return module_auto.common_request("ca9591623a91a8f8d196dbb31dccf7a6", ( index ) ,30)
    def get_load(self, index = 1):
        autoconnect()
        return module_auto.common_request("4b93f6b97a5801b7cb8ca217d323e975", ( index ) ,30)
    def stop(self, index = 1):
        autoconnect()
        return module_auto.common_request("5574099fb868a1819d4312e029a9e43b", ( index ) , 30)
motor_driver=motor_driver_c()

class speaker_c():
    def mute(self, index = 1):
        autoconnect()
        return module_auto.common_request("372733001e64f46f2ec17d7aa9238b2e", ( index ) ,30)
    def stop(self, index = 1):
        autoconnect()
        return module_auto.common_request("553e5b2ee1f27ca3b69f3c1077cd4a9b", ( index ) ,30)
    def set_vol(self, volume, index = 1):
        autoconnect()
        return module_auto.common_request("5e126dc66995384c55e659b3ff0b7082", ( volume, index ) , 30)
    def add_vol(self, volime, index = 1):
        autoconnect()
        return module_auto.common_request("f56a2f368ca8828e56e35e1e18c5377a", ( volime, index ) , 30)
    def get_vol(self, index = 1):
        autoconnect()
        return module_auto.common_request("49102cec80105f7ca1a56dd997664153", ( index ) ,30)
    def play_tone(self, frq, t = None, index = 1):
        autoconnect()
        return module_auto.common_request("7e0558574c28779fcd007636cecc0be4", ( frq, t , index ) , 30)
    def play_music(self, name, index = 1):
        autoconnect()
        return module_auto.common_request("d3bbc391371a4e7a99845969d527935f", ( name, index ) , 30)
    def play_music_until_done(self, name, index = 1):
        autoconnect()
        return module_auto.common_request("67ba91bacf4c08405663fa0a5aeadd8c", ( name, index ) ,30)
    def is_play(self, index = 1):
        autoconnect()
        return module_auto.common_request("7e3c5f3f01d88b7665a5adb092ac18a9", ( index ) ,30)
speaker=speaker_c()

class quad_rgb_sensor_c():
    def get_line_sta(self, index = 1):
        autoconnect()
        return module_auto.get_value("e7542a958616b5570860bafdfa6ebb99", ( index ))
    def get_offset_track(self, index = 1):
        autoconnect()
        return module_auto.get_value("1ba79ebbb614d451999c2e724be314c6", ( index ))
    def is_line(self, ch, index = 1):
        autoconnect()
        return module_auto.get_value("c2daa2a266efe45f23a90df25d3754ac", ( ch, index ))
    def is_background(self, ch, index = 1):
        autoconnect()
        return module_auto.get_value("a7e2fa1a8bb6cd00f61d9ae90cb5b697", ( ch, index ))
    def is_color(self, color = "white", ch = 1, index = 1):
        autoconnect()
        return module_auto.get_value("baa73a0c5d9989702b389038cd22a07b", ( color , ch , index ))
    def get_red(self, ch, index = 1):
        autoconnect()
        return module_auto.common_request("2ddf1b464752512ef3c0dbe43baffbe5", ( ch, index ) ,30)
    def get_green(self, ch, index = 1):
        autoconnect()
        return module_auto.common_request("17956f478f5fcd2dc77c96137c35ec20", ( ch, index ) ,30)
    def get_blue(self, ch, index = 1):
        autoconnect()
        return module_auto.common_request("a5f361960df16d800fc707b89fb4f23d", ( ch, index ) ,30)
    def get_gray(self, ch, index = 1):
        autoconnect()
        return module_auto.get_value("d7445014d8956fad6782b192532965fc", ( ch, index ))
    def reset(self, index=1):
        autoconnect()
        return module_auto.common_request("a24ce22610a995f693d02ddd352d6c8f", ( index) , 30)
    def get_color(self, ch, index = 1):
        autoconnect()
        return module_auto.get_value("7e363138275b67395fe41a416053f64b", ( ch, index ))
    def get_color_sta(self, ch, index = 1):
        autoconnect()
        return module_auto.get_value("32eec5621aadbbf1fec2fed254bdac56", ( ch, index ))
    def get_light(self, ch, index = 1):
        autoconnect()
        return module_auto.get_value("5ae8036f0061f4c060ec400dc06d6fd9", ( ch, index ))
    def set_led_on(self, id, color = 'white',index=1):
        autoconnect()
        return module_auto.common_request("47ef17b5a0ac04a718dfc0a4a4c63422", ( id, color ,index) , 30)
    def set_led(self, color = "white", index = 1):
        autoconnect()
        return module_auto.common_request("5cfb9592e2ef26e85b93fc180447fc37", ( color , index ) , 30)
    def set_led_off(self, id, index=1):
        autoconnect()
        return module_auto.common_request("bfe11a977243faa8fa3f12cee77f49df", ( id, index) , 30)
    def off_led(self, index = 1):
        autoconnect()
        return module_auto.common_request("ae821ef9234b7e5a6ce5e2f2cc10179c", ( index ) , 30)
    def set_line(self, index=1):
        autoconnect()
        return module_auto.common_request("c42e9df437c992f1203e4bcce563a3cb", ( index) , 30)
    def set_background(self, index=1):
        autoconnect()
        return module_auto.common_request("a3b215e69de27e092b65ec71ccdaaed4", ( index) , 30)
    def set_line_mode(self, mode=1,index = 1):
        autoconnect()
        return module_auto.common_request("3d6f808622bbc5ce5a084bb5b1da2c37", ( mode,index ) , 30)
quad_rgb_sensor=quad_rgb_sensor_c()

class ultrasonic2_c():
    def get(self, index = 1):
        autoconnect()
        return module_auto.get_value("a4bd4b007f893e1642ef5139e653fedd", ( index ))
    def led_show(self, bri_1, bri_2 = 0, bri_3 = 0, bri_4 = 0, bri_5 = 0, bri_6 = 0, bri_7 = 0, bri_8 = 0, index = 1):
        autoconnect()
        return module_auto.common_request("445e9f409b63afe18edaa3ccf6520105", ( bri_1, bri_2 , bri_3 , bri_4 , bri_5 , bri_6 , bri_7 , bri_8 , index ) , 30)
    def set_bri(self, led_bri, id, index =1):
        autoconnect()
        return module_auto.common_request("9f7c0ddc9d84a5a9d32b4f6f3a8c41b3", ( led_bri, id, index ) , 30)
    def add_bri(self, led_bri, id, index =1):
        autoconnect()
        return module_auto.common_request("a0d85bf7e33b182280383d3f61e6e178", ( led_bri, id, index ) , 30)
    def get_bri(self, id, index =1):
        autoconnect()
        return module_auto.common_request("d973b5d54cc2f8271e0727369d02cc66", ( id, index ) , 30)
    def play(self, emotion, index =1):
        autoconnect()
        return module_auto.common_request("d9875a5857f6726f3543ec7c014e0753", ( emotion, index ) , 30)
ultrasonic2=ultrasonic2_c()

class science_c():
    def pir_is_active(self, index=1):
        autoconnect()
        return module_auto.get_value("76de468370af9b36786b76f4b19fb392", ( index))
    def flame_is_active(self, index=1):
        autoconnect()
        return module_auto.get_value("6e0e36f0b5264d9d6a48bc8f3d4b7cff", ( index))
    def flame_get(self, index=1):
        autoconnect()
        return module_auto.get_value("13f6c66bafdd9ed79a11e862d2854887", ( index))
    def soil_get(self, index=1):
        autoconnect()
        return module_auto.get_value("2e5e9579589914e38ea65519ace764f1", ( index))
    def soil_get_resistance(self, index=1):
        autoconnect()
        return module_auto.get_value("bd1caebfa547d0b1468a3797998128df", ( index))
    def soil_get_adc_value(self, index=1):
        autoconnect()
        return module_auto.get_value("95e24c3ba3c36043a63295792b62cc1a", ( index))
    def mq2_is_active(self, index=1):
        autoconnect()
        return module_auto.get_value("4e9d21aeeb3197f6a881216170cdc37c", ( index))
    def mq2_get(self, index=1):
        autoconnect()
        return module_auto.get_value("264d1879d0cfdd6464d7f8bb924d93ae", ( index))
    def mq2_on(self, threshold, index=1):
        autoconnect()
        return module_auto.common_request("306453d56da17e477896824d476df8d9", ( threshold, index) , 30)
    def mq2_off(self, index=1):
        autoconnect()
        return module_auto.common_request("08120485df63137cebd247cb32f1f4ca", ( index) , 30)
    def touch_is_active(self, index=1):
        autoconnect()
        return module_auto.get_value("e2957c3bdbc7b3773c60e7184cb673a2", ( index))
    def touch_get_resistance(self, index=1):
        autoconnect()
        return module_auto.get_value("7e836077b0852bf2b4ad2627f6d3c8d5", ( index))
    def humiture_get_temp(self, index=1):
        autoconnect()
        return module_auto.get_value("f3ebd88d7bb9685da9530c89b58be69b", ( index))
    def humiture_get_humidity(self, index=1):
        autoconnect()
        return module_auto.get_value("3c90a384c4cb2a37f84be4694111aa05", ( index))
    def compass_get(self, axis,index = 1):
        autoconnect()
        return module_auto.get_value("fc6cc7dfea4537c79f7b9fc8b7930466", ( axis,index ))
    def compass_get_angle(self, index = 1):
        autoconnect()
        return module_auto.get_value("faba1b5cf273cbe2b7933414fcfa6291", ( index ))
    def compass_reset(self, switch,index = 1):
        autoconnect()
        return module_auto.common_request("953e3e07e5f441885c1fcba14f8290f1", ( switch,index ) , 30)
    def compass_is_active(self, index = 1):
        autoconnect()
        return module_auto.get_value("bddbbf44aef491aae52979bc8698028f", ( index ))
    def atmos_get(self, index=1):
        autoconnect()
        return module_auto.get_value("c72715b6579d04307c8772ef93c178d1", ( index))
    def atmos_get_altitude(self, index = 1):
        autoconnect()
        return module_auto.get_value("4d06aa4664388952a3053d870e1a39e2", ( index ))
science=science_c()

class smart_camera_c():
    def get_count(self, index = 1):
        autoconnect()
        return module_auto.common_request("2f97d006cd234ec9b57b0fc057dc962a", ( index ) , 30)
    def reset_count(self, index = 1):
        autoconnect()
        return module_auto.common_request("eff0ae01f00da114b0a4e8a160d3a132", ( index ) , 30)
    def set_mode(self, mode = "color", index = 1):
        autoconnect()
        return module_auto.common_request("9d7abe2f06cec422dff12afa1573aff9", ( mode , index ) , 30)
    def learn(self, sign = 1, t = "until_button", index = 1):
        autoconnect()
        return module_auto.common_request("fd716bc55bc291dadbdf05b9d179ae86", ( sign , t , index ) , 30)
    def detect_sign(self, sign, index = 1):
        autoconnect()
        return module_auto.common_request("da615504bdc03415d631e5ddd5f972f0", ( sign, index ) , 30)
    def detect_sign_location(self, sign, location, index = 1):
        autoconnect()
        return module_auto.common_request("d6335a4860fb122bd1e7e226aab4f596", ( sign, location, index ) , 30)
    def get_sign_x(self, sign, index = 1):
        autoconnect()
        return module_auto.common_request("ac811c338a185ca49d59f6061ff2dd18", ( sign, index ) , 30)
    def get_sign_y(self, sign, index = 1):
        autoconnect()
        return module_auto.common_request("555029a1cd05ab74f3ea0b6c5d44b0fb", ( sign, index ) , 30)
    def get_sign_wide(self, sign, index = 1):
        autoconnect()
        return module_auto.common_request("7a475b314472cfae51e12a9f97f078ce", ( sign, index ) , 30)
    def get_sign_hight(self, sign, index = 1):
        autoconnect()
        return module_auto.common_request("020210412e159546ce82c894664b6ed6", ( sign, index ) , 30)
    def open_light(self, index = 1):
        autoconnect()
        return module_auto.common_request("afd44209e8067ef506dd85f9c345abf9", ( index ) , 30)
    def close_light(self, index = 1):
        autoconnect()
        return module_auto.common_request("154afee2304de997ba7343fd960ceeb0", ( index ) , 30)
    def reset(self, index = 1):
        autoconnect()
        return module_auto.common_request("70fd6fcff886c22176d599d6b506e493", ( index ) , 30)
    def detect_label(self, label, index = 1):
        autoconnect()
        return module_auto.common_request("d55a71fa1ed7b80220016592e00cc7f0", ( label, index ) , 30)
    def get_label_x(self, label, index = 1):
        autoconnect()
        return module_auto.common_request("9a8973f53e5783263355c94311146448", ( label, index ) , 30)
    def get_label_y(self, sign, index = 1):
        autoconnect()
        return module_auto.common_request("be366580dd45813ec6c0dad478731ea8", ( sign, index ) , 30)
    def detect_cross(self, index = 1):
        autoconnect()
        return module_auto.common_request("0338fc84ad81df76160c7f3ee5efd71f", ( index ) , 30)
    def get_cross_x(self, index = 1):
        autoconnect()
        return module_auto.common_request("f4c2cb62a83d627683d7de5f382fce35", ( index ) , 30)
    def get_cross_y(self, index = 1):
        autoconnect()
        return module_auto.common_request("22733eeb612edd5194e9616c6d362dc6", ( index ) , 30)
    def get_cross_road(self, index = 1):
        autoconnect()
        return module_auto.common_request("11d79e2df0a20d650dd4a9faad3a8b12", ( index ) , 30)
    def get_cross_angle(self, sn = 1, index = 1):
        autoconnect()
        return module_auto.common_request("31f690b956d796d8acc1171a955c1073", ( sn , index ) , 30)
    def set_line(self, mode = "black", index = 1):
        autoconnect()
        return module_auto.common_request("d15458545f64480795e1c8a607dce926", ( mode , index ) , 30)
    def get_vector_start_x(self, index = 1):
        autoconnect()
        return module_auto.common_request("3d29d47571cffb09289844031f2e642b", ( index ) , 30)
    def get_vector_start_y(self, index = 1):
        autoconnect()
        return module_auto.common_request("a38492c3f38fbb50d784083d7d458787", ( index ) , 30)
    def get_vector_end_x(self, index = 1):
        autoconnect()
        return module_auto.common_request("dd8485e45a1335d481309800b557b72a", ( index ) , 30)
    def get_vector_end_y(self, index = 1):
        autoconnect()
        return module_auto.common_request("184ab213b139c1f5ec4cb24f2f50e6fe", ( index ) , 30)
    def set_vector_angle(self, angle, index = 1):
        autoconnect()
        return module_auto.common_request("9876f59082b10d5cb9ff227d6580acb1", ( angle, index ) , 30)
    def get_vector_angle(self, index = 1):
        autoconnect()
        return module_auto.common_request("9235b918cc92b3ff564a0ad82119e4dc", ( index ) , 30)
    def set_kp(self, kp, index = 1):
        autoconnect()
        return module_auto.common_request("bb3e59b6b7d7bb21a09a58d958289748", ( kp, index ) , 30)
    def get_sign_diff_speed(self, sign, axis, axis_val, index = 1):
        autoconnect()
        return module_auto.common_request("82db41994c40f625b08ea18f395178a9", ( sign, axis, axis_val, index ) , 30)
    def get_label_diff_speed(self, label, axis, axis_val, index = 1):
        autoconnect()
        return module_auto.common_request("c528dcf5d85889c6bb49508114f81d70", ( label, axis, axis_val, index ) , 30)
    def get_follow_vector_diff_speed(self, index = 1):
        autoconnect()
        return module_auto.common_request("84ace6214b0b14c20a9acc1431947308", ( index ) , 30)
    def is_lock_sign(self, sign, axis, axis_val, index = 1):
        autoconnect()
        return module_auto.common_request("1ec6126d268ed8fb8fe734a93dd7ef58", ( sign, axis, axis_val, index ) , 30)
    def is_lock_label(self, sign, axis, axis_val, index = 1):
        autoconnect()
        return module_auto.common_request("36baca791b38b9f586e224227e314a9f", ( sign, axis, axis_val, index ) , 30)
smart_camera=smart_camera_c()

class mbot2_c():
    def forward(self, speed = 50, t = "null"):
        autoconnect()
        return module_auto.common_request("255c7f0b6d80526ac902312355e9bf6d", ( speed , t ) , 30)
    def backward(self, speed = 50, t = "null"):
        autoconnect()
        return module_auto.common_request("e8af69599740f99e599091b7c18ee0f5", ( speed , t ) , 30)
    def turn_left(self, speed = 50, t = "null"):
        autoconnect()
        return module_auto.common_request("5f3a58c8a36b1f63c592412c2a5e3f1b", ( speed , t ) , 30)
    def turn_right(self, speed = 50, t = "null"):
        autoconnect()
        return module_auto.common_request("50380e3ddae93d289a109f7b61be81db", ( speed , t ) , 30)
    def straight(self, distance, speed = 50):
        autoconnect()
        return module_auto.common_request("b5734aefde75532c33ba8d07fa34da76", ( distance, speed ) , 30)
    def turn(self, angle, speed = 50):
        autoconnect()
        return module_auto.common_request("229147889a930e943746db7463eed34c", ( angle, speed ) , 30)
    def drive_power(self, EM1_power, EM2_power):
        autoconnect()
        return module_auto.common_request("6428049d7664388fcb4f5d36d58a89ae", ( EM1_power, EM2_power) , 30)
    def drive_speed(self, EM1_speed, EM2_speed):
        autoconnect()
        return module_auto.common_request("eb4828b2dc472994c9120bc61eb250b5", ( EM1_speed, EM2_speed) , 30)
    def EM_stop(self, port = "all"):
        autoconnect()
        return module_auto.common_request("5609fd5ea6dfa18e72abc72d719c6e03", ( port ) , 30)
    def EM_set_power(self, power, port):
        autoconnect()
        return module_auto.common_request("46296fbf9cb8f48bbe1f0fc20b61a0f5", ( power, port) , 30)
    def EM_set_speed(self, speed, port):
        autoconnect()
        return module_auto.common_request("5956d2dab9c1b2139584659c9d530db8", ( speed, port) , 30)
    def EM_turn(self, angle, speed, port):
        autoconnect()
        return module_auto.common_request("0bddbade243b0b31206944eb1bfb103b", ( angle, speed, port) , 30)
    def EM_get_angle(self, port):
        autoconnect()
        return module_auto.common_request("fadd7b62ec703bbded9b9b63d464e425", ( port) , 30)
    def EM_get_speed(self, port):
        autoconnect()
        return module_auto.common_request("9c703762b00091af8ecdf503fab52745", ( port) , 30)
    def EM_get_power(self, port):
        autoconnect()
        return module_auto.common_request("3b16b8c166cc3ddd641b1a9f1060cb5f", ( port) , 30)
    def EM_reset_angle(self, port):
        autoconnect()
        return module_auto.common_request("290e5b4eb94caa9c1db7bf75675337d4", ( port) , 30)
    def EM_lock(self, is_lock, port):
        autoconnect()
        return module_auto.common_request("4b74d77cb80bbbceb5d274cebaff43b0", ( is_lock, port) , 30)
    def motor_add(self, power, port):
        autoconnect()
        return module_auto.common_request("0b6f90f14173b1db264ad9cd373e528b", ( power, port) , 30)
    def motor_set(self, power, port):
        autoconnect()
        return module_auto.common_request("095961606cf3dd0ec42d3db289d5786f", ( power, port) , 30)
    def motor_get(self, port):
        autoconnect()
        return module_auto.common_request("71349d2aa8cd0b8c53ab3987aa0e1cae", ( port) , 30)
    def motor_drive(self, power1, power2):
        autoconnect()
        return module_auto.common_request("0ed68ff4dfe2e646062584d19891018b", ( power1, power2) , 30)
    def motor_stop(self, port):
        autoconnect()
        return module_auto.common_request("9ed08e9abe0d1404168d1c9d88162c0e", ( port) , 30)
    def servo_add(self, angle, port):
        autoconnect()
        return module_auto.common_request("6c1af1d3f9ae20e49bb3341d82245404", ( angle, port) , 30)
    def servo_set(self, angle, port):
        autoconnect()
        return module_auto.common_request("6a7e41ee5fb20edb4560ab6fa3989da4", ( angle, port) , 30)
    def servo_get(self, port):
        autoconnect()
        return module_auto.common_request("203be4e568d04bb069bc57da728a8269", ( port) , 30)
    def servo_release(self, port):
        autoconnect()
        return module_auto.common_request("fe99692fbd5bf8fe6d614bf240fda82d", ( port) , 30)
    def servo_drive(self, angle1, angle2, angle3, angle4):
        autoconnect()
        return module_auto.common_request("3adadc31e2441f1684ea5b76c9a9d7de", ( angle1, angle2, angle3, angle4) , 30)
    def led_on(self, r, g = 0, b = 0, id = "all", port = "all"):
        autoconnect()
        return module_auto.common_request("0d1f3e02a5f8a089c10b41b54ec37a20", ( r, g , b , id , port ) , 30)
    def led_show(self, color, port):
        autoconnect()
        return module_auto.common_request("25da975043ce4b377092d794e847de39", ( color, port) , 30)
    def led_move(self, step, cycle, port):
        autoconnect()
        return module_auto.common_request("2841281ca5b0ac5db32f0ed92e9101f0", ( step, cycle, port) , 30)
    def led_off(self, id = "all", port = 1):
        autoconnect()
        return module_auto.common_request("ce081fea87a0b57809f524afe862ec67", ( id , port ) , 30)
    def led_add_bri(self, brightness, port):
        autoconnect()
        return module_auto.common_request("f9488a6157b9def0e33bc092e60755a1", ( brightness, port) , 30)
    def led_set_bri(self, brightness, port):
        autoconnect()
        return module_auto.common_request("d5da19cec15c210edc40e2b2da5ce99e", ( brightness, port) , 30)
    def led_get_bri(self, port):
        autoconnect()
        return module_auto.common_request("28ee9973300e5f409243219427a26b4d", ( port) , 30)
    def write_digital(self, val, port ):
        autoconnect()
        return module_auto.common_request("8cc066584e3d183a657df5ea607ee22e", ( val, port ) , 30)
    def read_digital(self, port):
        autoconnect()
        return module_auto.common_request("93d8106a316d214ee0f46b184538c371", ( port) , 30)
    def set_pwm(self, duty, frequency, port):
        autoconnect()
        return module_auto.common_request("532f21a9893ef42812c47e007f2841de", ( duty, frequency, port) , 30)
    def read_analog(self, port):
        autoconnect()
        return module_auto.common_request("681e2600c3d4fff31540c8adcbcf99c6", ( port) , 30)
mbot2=mbot2_c()
