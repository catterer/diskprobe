# diskprobe
Simple resources accessebility checker.

## Build
Dependencies: boost

    mkdir .build ; cd .build
    cmake .. && make
    
## Config example

    [ faulty_test ]
    fail_randomly_with_probability = 10
    period_ms = 500
    on_failure = /usr/bin/osascript -e "display notification \"faulty_test failed\"" # MacOS notifications
    on_repair = /usr/bin/osascript -e "display notification \"faulty_test repaired\""
    
    [ file_writer ]
    write_smth_to_file = /tmp/t
    on_failure = notify-send "FS problems!"
    
## Allowed options:
    --help                                produce help message
    -c [ --config ] arg                   path to config file
    -o [ --out-file ] arg (=<stdout>)     path to log file
    -l [ --log-level ] arg (=2)           0-4, 0: only fatal, 4: everything
    -s [ --max-sampling-rate ] arg (=100) how often does main thread takes looks 
                                          at the clock, msec    
## Launch example
Usual
    dprobe -c cfg_file.ini
Daemonized
    nohup dprobe -c cfg_file.init -o /var/log/dprobe.log &
