#!/usr/bin/env python

from pcaspy import SimpleServer, Driver, Severity, Alarm
import requests
import time
import thread

prefix = 'SR00EMS01:'

pvdb = {
    'HUMIDITY_SENSOR_TEMPERATURE_MONITOR': {
        'type' : 'float',
        'unit' : 'C',
    },
    'PRESSURE_SENSOR_TEMPERATURE_MONITOR': {
        'type' : 'float',
        'unit' : 'C',
    },
    'TEMPERATURE_MONITOR': {
        'type' : 'float',
        'unit' : 'C',
    },
    'HUMIDITY_MONITOR': {
        'type' : 'float',
        'unit' : '%',
    },
    'PRESSURE_MONITOR': {
        'type' : 'float',
        'unit' : 'Pa',
    },
}

sensor_suffixes = pvdb.keys()

pvdb.update({
    'HOST_SP' : {
        'type' : 'string',
        'value' : '10.17.20.11',
    },
})


class IOC(Driver):
    def __init__(self):
        super(IOC, self).__init__()
        thread.start_new_thread(self.update,())
        self.read_error_processed = False

    def update(self):
        while True:
            try:
                url = 'http://{0}/'.format(self.getParam('HOST_SP'))
                response = requests.get(url,timeout=1)
                data = response.json()
            except Exception as error:
                if not self.read_error_processed:
                    for suffix in sensor_suffixes:
                        print suffix, error
                        self.setParamStatus(suffix, Alarm.READ_ALARM, Severity.INVALID_ALARM)
                    self.updatePVs()
                    self.read_error_processed = True
            else:
                for suffix in sensor_suffixes:
                    self.setParam(suffix, data[suffix])
                self.updatePVs()
                self.read_error_processed = False

            time.sleep(1)

if __name__ == '__main__':
    server = SimpleServer()

    server.createPV(prefix, pvdb)
    driver = IOC()

    while True:
        # process CA transactions
        server.process(0.1)
