#!/bin/bash
shopt -s expand_aliases
alias sqlite='/home/ec2-user/bin/sqlite-amalgamation-3240000/sqlite3'

create_db(){
   DB=/home2/ec2-user/dev/network_monitoring/stats.sqlite
   [[ -f $DB ]] &&  echo "$DB exists" >&2 && return 0 
   cat <<EOF|sqlite $DB
     create table host_stats(
               host         string     not null
              ,ip           string     not null  
              ,ts           int        not null default (strftime('%s','now'))
              ,rx           int        not null
              ,rxs          int        not null -- rx bytes/sec = (rx - prev_rx)/(ts - prev_ts)
              ,N            int        not null -- +=1
              ,mean         int        not null
              ,summ         int        not null
              /*
              ,old_rxsmean  int        not null
              ,new_rxsmean  int        not null
              ,old_rxssum   int        not null
              ,new_rxssum   int        not null
              */
              );
    create unique index i1 on host_stats(host); --IP may change for a host
EOF
}

push(){
  local value=$1
  local uptime_s=$(cat /proc/uptime | awk '{printf "%0.f", $1}')
  local rx_since_uptime=$(</sys/class/net/eth0/statistics/rx_bytes)
  cat <<EOF | sqlite -header -column -echo $DB
    create temp table old as 
    select host,ip,ts,rx,N,mean,summ 
    from host_stats 
    where host='$(hostname)';

    create temp table new (
       host string not null
      ,ip   string not null
      ,ts   int    not null default (strftime('%s','now'))
      ,rx   int    not null
    );

    insert into new (host,ip,rx) values ('$(hostname)', '$(hostname -i)',$value);

    create temp table updates as 
    select n.host
          ,n.ip
          ,n.ts
          ,$uptime_s as uptime_s
          ,n.rx
          ,ifnull(o.ts,strftime('%s','now')-$uptime_s ) as old_ts
          ,ifnull(o.rx,$rx_since_uptime)                as old_rx
          ,(n.rx - ifnull(o.rx,$rx_since_uptime) ) / (n.ts - ifnull(o.ts,strftime('%s','now')-$uptime_s) ) as rxs
          ,ifnull(o.N+1,1)                              as N
          ,o.mean + (n.rx - o.mean) / o.N+1             as mean
          --,o.summ + (n.rx - o.mean) * (n.rx - mean)   as summ
          --,ifnull(o.mean,0) + (n.rx - ifnull(o.mean,0))/ ifnull(o.N+1,1) as mean
          --,ifnull(o.summ,0) + (n.rx - ifnull(o.mean,0))*(n.rx - mean)
    from temp.new n LEFT OUTER JOIN temp.old o 
         ON n.host=o.host;

    UPDATE temp.updates
    SET mean = n.rx, summ = 0 
    WHERE N = 1;

    UPDATE temp.updates
    SET mean = n.rx, summ = 0 
    WHERE N = 1;
    
    REPLACE INTO host_stats (host,ip,ts,rx,rxs,N)
    SELECT host,ip,ts,rx,rxs,N 
    FROM temp.updates t
    ;

    select changes() as changes;
    /** THIS DIDNT WORK
    select * from temp.updates;
    insert into host_stats(host,ip,ts,rx,rxs,N)
    --values('ip-172-31-38-111','172.31.38.111',0,0,0,17)
    select host,ip,ts,rx,rxs,8
    from temp.updates 
    ON CONFLICT(host) do nothing --update set ts=998
    ;
    **/

    select * from host_stats;

    --insert into new (host,ip,ts,rx,prev_rx,cur_rx rxs,N,old_rxsmean,new_rxsmean,old_rxssum,new_rxssum)
EOF
}

create_db
push ${1:-$(</sys/class/net/eth0/statistics/rx_bytes)}

ls -lh $DB >&2

