#!/bin/env python3

MARKISOL_AGC1_PULSE = 4940 #5577 
MARKISOL_AGC2_PULSE = 2480
MARKISOL_AGC3_PULSE  = 1620 
MARKISOL_RADIO_SILENCE = 6000 

MARKISOL_PULSE_SHORT = 340 #300  
MARKISOL_PULSE_LONG = 680 

source = {
    'all': { 'open': '0110010101001010111100111000000010011100',
             'close': '0110010101001010111110001000000010010111',
             'stop': '0110010101001010111110101000000010010101' },
    'c01': { 'open': '0110010101001010100000111000000011100010',
             'close': '0110010101001010100010001000000011101111',
             'stop': '0110010101001010100010101000000011101101' },
    'c02': { 'open': '0110010101001010010000111000000001100010',
             'close': '0110010101001010010010001000000001101111',
             'stop': '0110010101001010010010101000000001101101' },
    'c03': { 'open': '0110010101001010110000111000000010100010',
             'close': '0110010101001010110010001000000010101111',
             'stop': '0110010101001010110010101000000010101101' },
    'c04': { 'open': '0110010101001010001000111000000000100010',
             'close': '0110010101001010001010001000000000101111',
             'stop': '0110010101001010001010101000000000101101' },
    'c05': { 'open': '0110010101001010101000111000000011000010',
             'close': '0110010101001010101010001000000011001111',
             'stop': '0110010101001010101010101000000011001101' },
    'c06': { 'open': '0110010101001010011000111000000001000010',
             'close': '0110010101001010011010001000000001001111',
             'stop': '0110010101001010011010101000000001001101' },
    'c07': { 'open': '0110010101001010111000111000000010000010',
             'close': '0110010101001010111010001000000010001111',
             'stop': '0110010101001010111010101000000010001101' },
    'c08': { 'open': '0110010101001010000100111000000000000010',
             'close': '0110010101001010000110001000000000001111',
             'stop': '0110010101001010000110101000000000001101' },
    'c09': { 'open': '0110010101001010100100111000000011111100',
             'close': '0110010101001010100110001000000011110111',
             'stop': '0110010101001010100110101000000011110101' },
    'c10': { 'open': '0110010101001010010100111000000001111100',
             'close': '0110010101001010010110001000000001110111',
             'stop': '0110010101001010010110101000000001110101' },
    'c11': { 'open': '0110010101001010110100111000000010111100',
             'close': '0110010101001010110110001000000010110111',
             'stop': '0110010101001010110110101000000010110101' },
    'c12': { 'open': '0110010101001010001100111000000000111100',
             'close': '0110010101001010001110001000000000110111',
             'stop': '0110010101001010001110101000000000110101' },
    'c13': { 'open': '0110010101001010101100111000000011011100',
             'close': '0110010101001010101110001000000011010111',
             'stop': '0110010101001010101110101000000011010101' },
    'c14': { 'open': '0110010101001010011100111000000001011100',
             'close': '0110010101001010011110001000000001010111',
             'stop': '0110010101001010011110101000000001010101' }

}



def output_timings(code):
    timings = []
    timings.append(-MARKISOL_RADIO_SILENCE)
    timings.append(MARKISOL_AGC1_PULSE)
    timings.append(-MARKISOL_AGC2_PULSE)
    timings.append(MARKISOL_AGC3_PULSE)

    for bit in code:
        if(bit == '0'): # LOW-HIGH-LOW
            timings.append(-MARKISOL_PULSE_SHORT) 
            timings.append(MARKISOL_PULSE_SHORT)
            timings.append(-MARKISOL_PULSE_SHORT)
        else:           # LOW-HIGH-HIGH
            timings.append(-MARKISOL_PULSE_SHORT)
            timings.append(MARKISOL_PULSE_LONG)

    timings.append(-MARKISOL_RADIO_SILENCE)

    def sign(n):
        if int(n) > 0:
            return "+"
        else:
            return "-"

    # Combine consecutive common records
    out = []
    acc = 0
    for this_t in timings:
        #print(this_t, acc)
        #print(sign(this_t), sign(acc), sign(this_t) == sign(acc))
        if acc == 0 or sign(this_t) == sign(acc):
            acc = acc + this_t
        else:
            out.append(acc)
            acc = this_t
    out.append(acc)    

    return(", ".join(map(str, out)))


for k, v in source.items():
    print('''
  - platform: template
    assumed_state: true
    optimistic: true
    name: blind_{name}
    close_action:
      - remote_transmitter.transmit_raw:
          code: [ {close_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {close_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {close_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {close_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {close_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {close_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {close_code} ]
    stop_action:
      - remote_transmitter.transmit_raw:
          code: [ {stop_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {stop_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {stop_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {stop_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {stop_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {stop_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {stop_code} ]
    open_action:
      - remote_transmitter.transmit_raw:
          code: [ {open_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {open_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {open_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {open_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {open_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {open_code} ]
      - remote_transmitter.transmit_raw:
          code: [ {open_code} ]
'''.format( name=k, friendly_name=k, close_code=output_timings(v['close']),stop_code=output_timings(v['stop']),open_code=output_timings(v['open']))
    )