#include "stationary_main.h"
#include "configurations.h"
#include "adc_isr.h"
#include "constants.h"
#include "dac_driver.h"
#include "relay.h"
#include "fourier.h"
#include "printing.h"
#include "utils.h"
#include "listener.h"
#include "tcp_client.h"
#include <Arduino.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <IntervalTimer.h>


StationaryMain::StationaryMain(config_t* config, Listener* listener){
    frequency_magnitudes = get_frequency_magnitudes();
    last_reading = get_last_reading();
    fourier_counter = get_fourier_counter();
    this->config = config;
    this->listener= listener;
}

void StationaryMain::setup(TcpClient* client){
    this->client = client;

    pinMode(NO_LEAK_PIN, INPUT);
    dac_setup(DAC_PIN, DAC_CLR_PIN, HV_ENABLE_PIN);
    
    setup_relay();
    switch_relay_to_receive();
    client->print("Setup relay\n");

    adc_timer.begin(adc_timer_callback, ADC_PERIOD);
    adc_setup();
    client->print("Setup adc and timer\n");

    fourier_initialize(config->fourier_window_size);
    client->print("Setup fourier\n");

    listener->begin(micros() + config->period);
}

void StationaryMain::send_mode_hb(){
    if (micros() >= ts_start_talking){
        reply_yell();
    }
}

void StationaryMain::reply_yell(){
    if (micros() - ts_start_talking < config->micros_send_duration){ // keep sending
        uint16_t my_freq = get_freq(config->my_frequency_idx);
        dac_set_analog_float(sinf(2 * M_PI * my_freq / 1000000 * (float)(micros() % (1000000 / my_freq))));
    } else { // finished beep
        ts_start_listening = micros() + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD;
        listener->begin(ts_start_listening);

        is_currently_receiving = true; // switch to receiving

        // client->print("sent for " + uint64ToString(micros() - ts_start_talking) + 
        //     "us. Finished sending at " + uint64ToString(micros()) + 
        //     ". Will start listening at " + uint64ToString(ts_start_talking) + "\n");
        switch_relay_to_receive();
        adc_timer.begin(adc_timer_callback, ADC_PERIOD);
    }
}

void StationaryMain::loop(){
    if(digitalRead(NO_LEAK_PIN) == THERE_IS_A_LEAK){
        digitalWrite(HV_ENABLE_PIN, LOW);    // turn off high voltage
        switch_relay_to_receive();    // switch to receive mode
        adc_timer.end(); // turn off ADC timer so that we only send Leak Detect messages
        client->send_leak_detected_panic_message();
    } else {
        String message= "";
        if (client->has_cmd_available()){
            message = client->get_incoming_cmd();
        }
        else if (Serial.available() > 0){
            message = Serial.readStringUntil(message_terminator);
        }
        if (message.length() > 0){

            // Split the message into tokens using the delimiter
            int pos = 0;
            while (pos < message.length()) {
                int next_pos = message.indexOf(message_delimiter, pos);
                if (next_pos == -1) {
                    next_pos = message.length();
                }
                String token = message.substring(pos, next_pos);
                pos = next_pos + 1;

                // Extract the field value based on the token prefix
                if (token.startsWith("h")){    // just a ping
                    client->print("hi\n");
                } else if (token.startsWith("s")){// stop trying to detect frequencies;
                    listen_for_call_and_respond = false;
                    switch_relay_to_receive();
                    adc_timer.begin(adc_timer_callback, ADC_PERIOD);
                    client->print("Stopped\n");
                } else if (token.startsWith("g")) { // start trying to detect frequencies
                    listen_for_call_and_respond = true;
                    switch_relay_to_receive();
                    adc_timer.begin(adc_timer_callback, ADC_PERIOD);
                    is_peak_finding = false;
                    is_currently_receiving = true;
                    client->print("Started\n");

                } else if (token.startsWith("f")) {    // change my frequency
                    config->my_frequency_idx = (uint16_t)token.substring(1).toInt();
                    client->print("Changed my frequency to " + String(config->my_frequency_idx) + "\n");

                } else if (token.startsWith("N")) {    // change window size
                    config->fourier_window_size = (uint16_t)token.substring(1).toInt();
                    config->duration_to_find_peak = ADC_PERIOD * config->fourier_window_size;
                    config->micros_send_duration = ADC_PERIOD * config->fourier_window_size;
                    client->print("Changed window size to " + String(config->fourier_window_size) + "\n");

                } else if (token.startsWith("t")) {
                    config->dft_threshold = (uint32_t)token.substring(1).toInt();
                    client->print("Changed DFT threshold to " + String(config->dft_threshold) + "\n");

                } else if (token.startsWith("r")) { // use rising edge 
                    uint8_t use_rising_edge = (uint8_t)token.substring(1).toInt();
                    config->use_rising_edge = use_rising_edge > 0;
                    client->print("Use rising edge set to " + String(config->use_rising_edge) + "\n");
                }
            }
        }

        if (listen_for_call_and_respond){
            if (is_currently_receiving){
                listener_output_t listener_data = listener->hb();
                if (listener_data.finished){
                    if (listener_data.idx_identified_freq == config->my_frequency_idx){
                        is_currently_receiving = false;
                        ts_start_talking = listener_data.ts_peak + config->period;
                        adc_timer.end();
                        switch_relay_to_send();
                    }
                    else{
                        listener->begin(listener_data.ts_peak + config->period - MICROS_TO_LISTEN_BEFORE_END_OF_PERIOD);
                    }
                }
            } else {    // sending
                send_mode_hb();
            }
        }
        
        if (micros() - t_last_printed > 1000000){
            client->print(uint64ToString(*fourier_counter) + " Hz, Last Value: " + 
                String(*last_reading) + ", magnitudes: [" + 
                String(frequency_magnitudes[0], 0) + ", " + String(frequency_magnitudes[1], 0) + ", " + 
                String(frequency_magnitudes[2], 0) + ", " + String(frequency_magnitudes[3], 0) + ", " + 
                String(frequency_magnitudes[4], 0) + ", " + String(frequency_magnitudes[5], 0) + "]\n");
            t_last_printed = micros();
            *fourier_counter = 0;
        }
    }
}
