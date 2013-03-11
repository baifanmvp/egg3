#!/usr/bin/perl

my %rws = ();
my @eggd = ();
my %cluster_host = ();
my %cluster_host_egg = ();
my %cluster_egg = ();

my %eggdirs = ();

my %ip_rws = ();
my %ip_cluster = ();
my %ip_eggd = ();

my %ip_dirs = ();

my %ip_username = ();

my $basedir = "";


my $cfgfile;
#open(my $cfgfile, "deploy.cfg");
unless (open($cfgfile, $ARGV[0]))
{
    printf STDERR "Usage: $0 file.cfg\n";
    printf STDERR "\n";
    printf STDERR "\n";
    
    die "open cfg: $!";
}


while (<$cfgfile>)
{
    chomp;
    if (/^$/)
    {
        next;
    }
    
    my ($name, $val) = split('=');
    if ($name eq "basedir")
    {
        $basedir = $val;
    }
    elsif ($name =~ /^user/)
    {
        my ($_trash_, $id, $na_) = split("[.]", $name);
        $ip_username{$id}{$na_} = $val;
    }
    elsif ($name =~ /^rws/)
    {
        my ($_trash_, $rws_name, $rws_key) = split("[.]", $name);
        $rws{$rws_name}{$rws_key} = $val;
#	printf "$rws_name $rws_key: %s\n", 	$rws[$rws_name][$rws_key];

    }
    elsif ($name =~ /^eggd/)
    {
        my ($_trash_, $eggd_name, $eggd_key) = split("[.]", $name);
        $eggd[$eggd_name]{$eggd_key} = $val;
    }
    elsif ($name =~ /^cluster/)
    {
        my ($_trash_, $f1, $f2, $f3, $f4) = split("[.]", $name);
        if ($f1 =~ /^[0-9]+$/ && $f2 eq "ip")
        {
            $cluster_host[$f1]{ip} = $val;
        }
        elsif ($f1 =~ /^[0-9]+$/)
        {
            if ($f4 eq "ip")
            {
                $cluster_host_egg[$f1]{$f2}{$f3}{ip} = $val;
            }
            elsif ($f4 eq "range")
            {
                $cluster_host_egg[$f1]{$f2}{$f3}{range} = $val;
            }
        }
        elsif ($f1 !~ /^[0-9]+$/)
        {
            $cluster_egg{$f1}{$f2} = $val;
        }
        
    }
}
close($cfgfile);


sub build_rws {

    foreach my $rwsname (keys %rws)
    {
	my @eggdbs = split(",", $rws{$rwsname}{eggdb});
	delete $rws{$rwsname}{eggdb};
	
	my $i = 1;
	foreach my $_eggdb (@eggdbs)
	{
	    $_eggdb =~ s/\$\{([^}]+)\}/$1/;
	    my ($name_, $no_, $ip_) = split("[.]", $_eggdb);

	    if ($name_ eq "cluster")
	    {
		$rws{$rwsname}{eggdb}[$i] = $cluster_host[$no_]{ip};
		$i++;
	    }
	}
    }

}

build_rws;

sub gen_rws {
    open(my $f, ">", "rws-eggd.cfg.127.0.0.1")
	or die "open rws-eggd.cfg: $!";
    
    printf $f <<"EOF";
<RWS>
EOF
;
    foreach my $name_ (keys %rws)
    {
	printf $f <<EOF;
<$name_>
port=$rws{$name_}{port}
ip=$rws{$name_}{ip}
#eggDB address
EOF
;

	my $i = 1;
	foreach my $ip_ (@{$rws{$name_}{eggdb}})
	{
	    if (defined ($ip_))
	    {
		printf $f "cluster://%s:8888/%s_%d\n",
		$ip_, $name_, $i;
		$i++;
	    }
	}
	
	printf $f <<EOF;
# Server开始后，启动#个线程，处理连接请求。
# 默认0，线程数# = baker egg的个数。
connectthreadnum=0

logfile=$basedir$name_/EGGRWS.LOG

# 在workdir目录下建立eggMemServer、eggDocExport所用的数据
workdir=$basedir$name_/

# 新的数据先加入eggMemServer。eggMemServer将数据只放在内存。
# 如果不填将忽略eggDocument_add操作
memserverexename=/usr/local/bin/eggRWSPackageSaver

# 每隔#分钟生成一个eggMemServer
# memserverage=1d 为1天。 h为小时。
# 默认0，=1d
#min
memserverage=1

# eggMemServer的个数限制，超过时写入阻塞。至少要>=5
nummemservermax=40

# 将eggMemServer的内存数据同步到baker egg上
docexportexename=/usr/local/bin/eggRWSExportDoc

counter=no
</$name_>

EOF
;
    }

    printf $f <<EOF;
</RWS>
EOF
;
    close($f);
    
}

sub gen_eggd {
    my $i;
    for ($i = 1; $i <= $#eggd; $i++)
    {
	if (defined($eggd[$i]))
	{
	    open(my $f, ">", "eggd.cfg.$eggd[$i]{ip}")
		or die "open eggd.cfg.$eggd[$i]{ip}: $!";
	    printf $f <<"EOF";
<SOCKD>
socket=/tmp/egg3.sock
ip=$eggd[$i]{ip}
port=10000

logpath=/var/log/egg3/eggd.log
#0:debug 1:info 2:warn 3:error 4:claim
loglevel=1

</SOCKD>

EOF
;
	    close($f);
	}
    }
}

sub build_cluster {

    foreach my $_eggname (keys %cluster_egg)
    {
	my ($start, $step, $count) = ();
	$start = $cluster_egg{$_eggname}{start};
	$step = $cluster_egg{$_eggname}{step};
	$count = $cluster_egg{$_eggname}{count};
	undef $cluster_egg{$_eggname};
	my $i;
	for ($i = 1; $i <= $count; $i++)
	{
	    $cluster_egg{$_eggname}[$i] = $start;
	    $start += $step;
	    $cluster_egg{$_eggname}[$i+1] = $start;
	}
    }

    my $i;
    for ($i = 1; $i <= $#cluster_host_egg; $i++)
    {
	if (! defined ($cluster_host_egg[$i]))
	{
	    next;
	}
	foreach my $name_ (keys %{$cluster_host_egg[$i]})
	{
	    my $ranges = $cluster_host_egg[$i]{$name_};
	    undef $cluster_host_egg[$i]{$name_};

	    foreach my $i_range (keys %{$ranges})
	    {
		my $ip_ = ${$ranges}{$i_range}{ip};
		$ip_ =~ s/\$\{([^}]+)\}/$1/;
		my ($eggd_, $no_, $ip_) = split("[.]", $ip_);
		$ip = $eggd[$no_]{ip};
		
		my $r = ${$ranges}{$i_range}{range};
		my ($s, $e) = split('-', $r);
		while ($s <= $e)
		{
		    my $t1 = $cluster_egg{$name_}[$s];
		    my $t2 = $cluster_egg{$name_}[$s+1] - 1;
		    if ($s == $#{$cluster_egg{$name_}} - 1)
		    {
			$cluster_host_egg[$i]{$name_}[$s]{range} = "[$t1, ]";
		    }
		    else
		    {
			$cluster_host_egg[$i]{$name_}[$s]{range} = "[$t1, $t2]";
		    }

		    $cluster_host_egg[$i]{$name_}[$s]{ip} = $ip;

		    $cluster_host_egg[$i]{$name_}[$s]{dir}
		    = "${basedir}${name_}/${name_}_${i}/$t1/";

		    $cluster_host_egg[$i]{$name_}[$s]{start} = "$t1";
		    
		    $cluster_host_egg[$i]{$name_}[$s]{body}
		    = "tcp://${ip}:10000/%%%${basedir}${name_}/${name_}_${i}/$t1/";


		    $s++;
		}
	    }
	}
    }

}

build_cluster;
sub gen_cluster {
    my $i_cluster;
    for ($i_cluster = 1; $i_cluster <= $#cluster_host; $i_cluster++)
    {
	if (!defined($cluster_host[$i_cluster]))
	{
	    next;
	}
	my $ip = ${$cluster_host[$i_cluster]}{ip};

	open(my $f, ">", "cluster-eggd.cfg_${i_cluster}.$ip")
	    or die "open cluster-eggd.cfg_${i_cluster}.$ip: $!";
	
	printf $f <<"EOF";
<CLUSTER>

logpath=/var/log/egg3/cluster-eggd.log

#0:debug 1:info 2:warn 3:error 4:claim
loglevel=1


listen $ip:8888

#eggDirPath
#range hostAddress

#eggDirPath match RegExp: "^[^[ ][^ ]*"
#range is integer: "[range.start,range.end]"
#hostAddress is directory path on host: "type:ip:port:path"
#type: one of {fcgi, tcp}

EOF
;
	
	
	foreach my $egg_name (keys %{$cluster_host_egg[$i_cluster]})
	{
	    printf $f <<"EOF";
<${egg_name}_${i_cluster}>
EOF
;

	    my $i_rb;
	    my $rb_ = $cluster_host_egg[$i_cluster]{$egg_name};

	    for ($i_rb = 1; $i_rb <= $#{$rb_}; $i_rb++)
	    {
		printf $f "%s %s\n",
		${$rb_}[$i_rb]{range},
		${$rb_}[$i_rb]{body};
	    }
	    
	    printf $f <<"EOF";
</${egg_name}_${i_cluster}>
EOF
;
	}
	    
	printf $f <<"EOF";

</CLUSTER>
EOF
;
	close($f);
    }

    
}

sub build_eggdirs {
    my $i_cluster;
    for ($i_cluster = 1; $i_cluster <= $#cluster_host; $i_cluster++)
    {
	if (!defined($cluster_host[$i_cluster]))
	{
	    next;
	}
	foreach my $egg_name (keys %{$cluster_host_egg[$i_cluster]})
	{
	    my $i_rb;
	    my $rb_ = $cluster_host_egg[$i_cluster]{$egg_name};

	    for ($i_rb = 1; $i_rb <= $#{$rb_}; $i_rb++)
	    {
		my $id_ = ${$rb_}[$i_rb]{start};
		my $ip_ = ${$rb_}[$i_rb]{ip};
		my $dir_ = ${$rb_}[$i_rb]{dir};
		$eggdirs{$egg_name}{$id_}[$i_cluster]{ip} = $ip_;
		$eggdirs{$egg_name}{$id_}[$i_cluster]{dir} = $dir_;
	    }
	    
	}
    }
}
build_eggdirs;


sub build_ipservers {

    my $i;
    
    for ($i = 1; $i <= $#cluster_host; $i++)
    {
        if (!defined($cluster_host[$i]))
        {
            next;
        }
        my $ip_;
        $ip_ = $cluster_host[$i]{ip};
        $ip_cluster{$ip_} = "info";
    }

    for ($i = 1; $i <= $#eggd; $i++)
    {
        if (!defined($eggd[$i]))
        {
            next;
        }
        my $ip_;
        $ip_ = $eggd[$i]{ip};
        $ip_eggd{$ip_} = "svc";
    }

    foreach my $name_ (keys %rws)
    {
        my $ip_;
        $ip_ = $rws{$name_}{ip};
        $ip_rws{$ip_} = "rws";
    }
}
build_ipservers;


sub build_usernames {
    foreach my $name_ (keys %ip_username)
    {
        my $ip = $ip_username{$name_}{ip};
        my $name = $ip_username{$name_}{name};
        undef $ip_username{$name_};
        $ip_username{$ip} = $name;
    }
}
build_usernames;


sub gen_rwsctl {

    open(my $f, ">", "rws-eggd-ctl.cfg.127.0.01")
        or die "open rws-eggd-ctl.cfg.127.0.01: $!";


    foreach my $ip_ (keys %ip_eggd)
    {
        printf $f "%s %s %s\n", $ip_username{$ip_}, $ip_, $ip_eggd{$ip_};
    }
    foreach my $ip_ (keys %ip_cluster)
    {
        printf $f "%s %s %s\n", $ip_username{$ip_}, $ip_, $ip_cluster{$ip_};
    }
    foreach my $ip_ (keys %ip_rws)
    {
        printf $f "%s %s %s\n", $ip_username{$ip_}, $ip_, $ip_rws{$ip_};
    }
    printf $f "\n";    
    
    foreach my $name_ (keys %eggdirs)
    {
        foreach my $id_ (keys %{$eggdirs{$name_}})
        {
            if (!defined($id_))
            {
                next;
            }

            printf $f "[%s_%s]\n", $name_, $id_;
            
            my $i_;
            for ($i_ = 1; $i_ <= $#{$eggdirs{$name_}{$id_}}; $i_++)
            {
                printf $f "%s %s\n",
                $eggdirs{$name_}{$id_}[$i_]{ip},
                $eggdirs{$name_}{$id_}[$i_]{dir};
            }
            printf $f "\n";
        }
    }
    close($f);
}

sub build_dirs {
    
    foreach my $name_ (keys %eggdirs)
    {
	foreach my $id_ (keys %{$eggdirs{$name_}})
	{
	    if (!defined($id_))
	    {
		next;
	    }

	    my $i_;
	    for ($i_ = 1; $i_ <= $#{$eggdirs{$name_}{$id_}}; $i_++)
	    {
		my $ip_;
		my $dir_;
		$ip_ = $eggdirs{$name_}{$id_}[$i_]{ip},
		$dir_ = $eggdirs{$name_}{$id_}[$i_]{dir};
		
		$ip_dirs{$ip_}{$dir_} = 1;
	    }
	}
    }
}
build_dirs;


sub gen_dirs {

    foreach my $ip_ (keys %ip_dirs)
    {
	open(my $f, ">", "mkdir.$ip_")
	    or die "open mkdir.$ip_: $!";

	foreach my $dir_ (keys %{$ip_dirs{$ip_}})
	{
	    printf $f "%s\n", $dir_;
	}
	
	close($f);
    }
    
    foreach my $rws_name (keys %rws)
    {
	my $ip_ = $rws{$rws_name}{ip};
	open(my $f, ">>", "mkdir.$ip_")
	    or die "open mkdir.$ip_: $!";
	printf $f "${basedir}%s\n", $rws_name;
	close($f);
    }
}


#-----------------------------

gen_rws;
gen_eggd;
gen_cluster;
gen_rwsctl;
gen_dirs;





=doc
# sample deploy.cfg

basedir=/ape/egg3data/bas2db/

user.a.ip=172.168.1.20
user.a.name=ape
user.b.ip=172.168.1.28
user.b.name=bf

cluster.1.ip=172.168.1.20
cluster.2.ip=172.168.1.28

cluster.news.start=1325347200
cluster.news.step=604800
cluster.news.count=65
cluster.1.news.a.ip=${eggd.1.ip}
cluster.1.news.a.range=1-65
cluster.2.news.a.ip=${eggd.2.ip}
cluster.2.news.a.range=1-65

eggd.1.ip=172.168.1.20
eggd.2.ip=172.168.1.28

rws.news.ip=172.168.1.20
rws.news.port=12001
rws.news.eggdb=${cluster.1.ip},${cluster.2.ip}

=cut
