# -*- coding: utf-8 -*
import json
import makeblock.utils
from makeblock.protocols.PackData import HalocodePackData
import time

class _BaseModule:
    def __init__(self,board,index=0):
        self._pack = None
        self.setup(board,index)
        
    def _callback(self,data):
        pass

    def setup(self,board,index):
        board._autoconnect()
        self._board = board
        self._index = index
        self._init_module()
    
    def _init_module(self):
        pass

    def request(self,pack):
        self._board.remove_response(pack)
        self._board.request(pack)

    def call(self,pack):
        self._board.call(pack)
    
    def send_script_old(self,mode,script):
        '''
            发送py代码
        '''
        self._pack.mode = mode
        self._pack.script = script
        if mode==HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE:
            self.call(self._pack)
        if mode==HalocodePackData.TYPE_RUN_WITH_RESPONSE:
            self.request(self._pack)

    def send_script(self, pack):
        '''
            发送py代码
        '''
        if pack.mode==HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE:
            self.call(pack)
        if pack.mode==HalocodePackData.TYPE_RUN_WITH_RESPONSE:
            self.request(pack)
        if pack.mode==HalocodePackData.TYPE_RUN_IMDT_WITE_RESPONSE:
            self.request(pack)

    def subscribe(self,pack):
        '''
            订阅数据
        '''
        self._board.subscribe(pack)

    def unsubscribe(self,pack):
        '''
            取消订阅
        '''
        self._board.unsubscribe(pack)

GET_VALUE_DELAY_TIME = 0.001
REQUEST_DELAY_TIME = 0.001

from makeblock.modules.cyberpi import table_cyberpi_api
class BaseModuleAuto(_BaseModule):
    def _init_module(self):
        self._pack = HalocodePackData()
        self._pack.type = HalocodePackData.TYPE_SCRIPT
        self._pack.on_response = self.common_request_response_cb
        
        self.pac_table = table_cyberpi_api.table_tag
#################################################################################
    def create_request_str(self, item):
        func_script = item.func
        para = str(item.paras)
        return func_script + para

    def create_subscribe_str(self, item):
        key = "{0}"
        func_script = item.func
        para = item.paras
        return "subscribe.add_item(%s, %s, %s)" %(key, func_script, para)

    
    def wait_respond(self, pack, max_time_s = 3):
        pack.data_new_flag = False
        start_time = time.time()
        while True:
            time.sleep(0.01)
            if pack.data_new_flag == True:
                self._board.remove_response(pack)
                return True
            if time.time() - start_time > max_time_s:
                self._board.remove_response(pack)
                return False

    # pack is a new object
    def common_subscribe_response_cb(self, pack):
        resp = self._board.find_subscribe_response(pack)
        if not resp is None:
            resp.subscribe_value = pack.subscribe_value 
            resp.subscribe_key = pack.subscribe_key
            resp.data_new_flag = True
            if resp._event_cb:
                resp._event_cb(resp)

    def common_request_response_cb(self, pack):
        resp = self._board.find_response(pack)
        try:
            # ret = eval("".join([ chr(i) for i in pack.data[3:len(pack.data)]]))
            ret = eval(str(bytes(pack.data[3:len(pack.data)]), 'utf-8'))
            resp.request_value = ret['ret']
            resp.data_new_flag = True
        except:
            resp.request_value = None
            print("error")
    
    def register_package_table(self, pac_table):
        self.pac_table.update(pac_table)

    def delay_sync(self, t = 0):
        if t > 0:
            time.sleep(t)

    '''
    subscribe - publish communication 
    '''
    def get_value(self, tag, para = None, event_cb = None):
        if not isinstance(para, tuple):
            para = (para, )

        self.delay_sync(GET_VALUE_DELAY_TIME)

        if tag in self.pac_table:
            if self.pac_table[tag]["pac"] == None or (not para in self.pac_table[tag]["pac"]):
                self.pac_table[tag]["cell"].update_para(para)

                _pack = HalocodePackData()
                _pack.type = HalocodePackData.TYPE_SCRIPT
                _pack.script = self.create_subscribe_str(self.pac_table[tag]["cell"])
                _pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
                _pack.on_response = self.common_subscribe_response_cb
                _pack._event_cb = event_cb

                self.subscribe(_pack)
                
                if self.wait_respond(_pack, 1):
                    if self.pac_table[tag]["pac"] == None:
                        self.pac_table[tag]["pac"] = {}
                    self.pac_table[tag]["pac"].update({para: _pack}) 
                else:
                    # subscribe command will resend
                    pass


                return _pack.subscribe_value
            else:
                return self.pac_table[tag]["pac"][para].subscribe_value

    '''
    Real-time communication
    '''
    def common_request(self, tag, para = None, wait_time = None, w_mode = None):
        if not isinstance(para, tuple):
            para = (para, )

        self.delay_sync(REQUEST_DELAY_TIME)

        if tag in self.pac_table:
            if not self.pac_table[tag]["pac"]:
                self.pac_table[tag]["cell"].update_para(para)
                _pack = HalocodePackData()
                _pack.type = HalocodePackData.TYPE_SCRIPT
 
                _pack.script = self.create_request_str(self.pac_table[tag]["cell"])
                _pack.on_response = self.common_request_response_cb
                
                if wait_time:
                    if w_mode:
                        _pack.mode = w_mode
                    else:
                        _pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
                    self.send_script(_pack)
                    self.wait_respond(_pack, wait_time)
                    try:
                        if not (_pack.request_value is None):
                            return _pack.request_value
                        else:
                            return None
                    except:
                        return None
                else:
                    if w_mode:
                        _pack.mode = w_mode
                    else:
                        _pack.mode = HalocodePackData.TYPE_RUN_WITHOUT_RESPONSE
                    self.send_script(_pack)

    '''
    subscribe - publish communication 
    '''
    def register_event(self, tag, para = None, event_cb = None):
        if not isinstance(para, tuple):
            para = (para, )

        print(tag, para, event_cb)
        self.delay_sync(GET_VALUE_DELAY_TIME)

        if tag in self.pac_table:
            if self.pac_table[tag]["pac"] == None or (not para in self.pac_table[tag]["pac"]):
                self.pac_table[tag]["cell"].update_para(para)

                _pack = HalocodePackData()
                _pack.type = HalocodePackData.TYPE_SCRIPT
                _pack.script = self.create_subscribe_str(self.pac_table[tag]["cell"])
                _pack.mode = HalocodePackData.TYPE_RUN_WITH_RESPONSE
                _pack.on_response = self.common_subscribe_response_cb
                _pack._event_cb = event_cb

                self.subscribe(_pack)
                
                if self.wait_respond(_pack, 1):
                    if self.pac_table[tag]["pac"] == None:
                        self.pac_table[tag]["pac"] = {}
                    self.pac_table[tag]["pac"].update({para: _pack}) 
                    return _pack.subscribe_key
                else:
                    # subscribe command will resend
                    return -1
            else:
                self.pac_table[tag]["pac"][para]._event_cb = event_cb
                return self.pac_table[tag]["pac"][para].subscribe_key
        return -2


    '''
    special usage
    '''
    def request_with_origin_script(self, script, wait_time = None):

        self.delay_sync(REQUEST_DELAY_TIME)
