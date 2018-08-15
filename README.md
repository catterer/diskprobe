# diskprobe
Simple resources accessebility checker.

## Build
Dependencies: boost

    mkdir .build ; cd .build
    cmake .. && make
    
## Config example

    [ faulty_test ]
    fail_randomly_with_probability = 10

    # launch every 500 msec; optional; default is 1000
    period_ms = 500

    # do this when task starts to fail/timeout
    on_failure = /usr/bin/osascript -e "display notification \"faulty_test failed\""

    # do this when the task stops to fail/timeout
    on_repair = /usr/bin/osascript -e "display notification \"faulty_test repaired\""

    # task timeouts after 500 ms
    task_time_limit_ms = 500
    

    [ file_writer ]
    write_smth_to_file = /tmp/t
    on_failure = notify-send "FS problems!"
    task_time_limit_ms = 500
    
    [ file_reader ]
    read_smth_from_file = /tmp/t
    on_failure = notify-send "FS problems!"
    task_time_limit_ms = 500
    
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
