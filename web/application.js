$(function() {
    if($("#indexHtml").length > 0) {
        fetch();
    }

    // For mqtt
    $('#m').on('change', function() {
        var inputs = $('.mc');
        inputs.prop('disabled', !$(this).is(':checked'));
    });
    
    $('#f').on('change', function() {
        var val = parseInt($(this).val());
        if(val == 3) {
            $('.f3-s').show();
        } else {
            $('.f3-s').hide();
        }
    });

    $('#s').on('change', function() {
        var port = $('#p');
        if($(this).is(':checked')) {
            if(port.val() == 1883) {
                port.val(8883);
            }
        } else {
            if(port.val() == 8883) {
                port.val(1883);
            }
        }
    });

    $('#m').trigger('change');
    $('#f').trigger('change');
    // For wifi
    $('#st').on('change', function() {
        if($(this).is(':checked')) {
            $('#i').show();
        } else {
            $('#i').hide();
        }
    });
    $('#st').trigger('change');

    // For web
    $('#as').on('change', function() {
        var inputs = $('.ac');
        inputs.prop('disabled', $(this).val() == 0);
    });
    
    $('#as').trigger('change');

    // For file upload
    $('#fileUploadField').on('change',function(){
        var fileName = $(this).val();
        $(this).next('.custom-file-label').html(fileName);
    })

    // For NTP
    $('#n').on('change', function() {
        var inputs = $('.nc');
        inputs.prop('disabled', !$(this).is(':checked'));
    });
    $('#n').trigger('change');

    // Navbar
    switch(window.location.pathname) {
        case '/modbus':
        case '/wifi':
        case '/mqtt':
        case '/mqtt-ca':
        case '/mqtt-cert':
        case '/mqtt-key':
        case '/web':
        case '/ntp':
            $('#config-link').addClass('active');
            break;
        case '/gpio':
        case '/firmware':
        case '/reset':
            $('#system-link').addClass('active');
            break;
    }

    // TODO Check for software upgrade
    return;
    var swv = $('#swVersion')
    if(swv.length == 1 && swv.text() != "SNAPSHOT") {
        var v = swv.text().substring(1).split('.');
        var v_major = parseInt(v[0]);
        var v_minor = parseInt(v[1]);
        var v_patch = parseInt(v[2]);
        $.ajax({
            url: swv.data('url'),
            dataType: 'json'
        }).done(function(releases) {
            releases.reverse();
            var me;
            var next_patch;
            var next_minor;
            var next_major;
            $.each(releases, function(i, release) {
                var ver2 = release.tag_name;
                var v2 = ver2.substring(1).split('.');
                var v2_major = parseInt(v2[0]);
                var v2_minor = parseInt(v2[1]);
                var v2_patch = parseInt(v2[2]);

                if(v2_major == v_major) {
                    if(v2_minor == v_minor) {
                        if(v2_patch > v_patch) {
                            next_patch = release;
                        }
                    } else if(v2_minor == v_minor+1) {
                        next_minor = release;
                    }
                } else if(v2_major == v_major+1) {
                    if(next_major) {
                        var mv = next_major.tag_name.substring(1).split('.');
                        var mv_major = parseInt(mv[0]);
                        var mv_minor = parseInt(mv[1]);
                        var mv_patch = parseInt(mv[2]);
                        if(v2_minor == mv_minor) {
                            next_major = release;
                        }
                    } else {
                        next_major = release;
                    }
                }
            });
            if(next_minor) {
                nextVersion = next_minor;
            } else if(next_major) {
                nextVersion = next_major;
            } else if(next_patch) {
                nextVersion = next_patch;
            }
            if(nextVersion) {
                $('#newVersionTag').text(nextVersion.tag_name);
                $('#newVersionUrl').prop('href', nextVersion.html_url);
                $('#newVersion').removeClass('d-none');
            }
        });
    }
});

var interval = 5000;
var fetch = function() {
    $.ajax({
        url: '/data.json',
        timeout: 10000,
        dataType: 'json'
    }).done(function(json) {
        for(var id in json) {
            if(!isNaN(id)) {
                id = parseInt(id);
            }
            
            var str = json[id];
            if(typeof str === "object") {
                continue;
            }

            $('.v'+id).each(function() {
                var $this = $(this);

                if($this.is('input[type=radio],input[type=checkbox]')) {
                    $this.prop('checked', $this.val() === str);
                    return true;
                }

                if($this.is('input,select')) {
                    $this.val(str);
                    return true;
                }

                var format = $this.data('format');

                if(format === '%d') {
                    $this.html(parseInt(str));
                    return true;
                }

                var match = /\%\.(\d+)f/.exec(format);
                if(match != null) {
                    var fractions = parseInt(match[1]);
                    var num = parseFloat(str);
                    $this.html(num.toFixed(fractions));
                    return true;
                }

                $this.html(str);
                return true;
            });

            $('.r'+id).show();
        }
        setTimeout(fetch, interval);
    }).fail(function(x, text, error) {
        console.log("Failed request");
        console.log(text);
        console.log(error);
        setTimeout(fetch, interval*3);

        setStatus("mqtt", 0);
        setStatus("wifi", 0);
        setStatus("modbus", 0);
        setStatus("esp", 3);
    });
};

$('input,select').on('change', function() {
    var $this = $(this);
    if($this.is('input[type=radio]') && !$this.is(':checked')) {
        return;
    }

    $.ajax({
        url: '/write.json',
        method: 'post',
        timeout: 10000,
        dataType: 'json',
        data: {
            id: $this.attr("name"),
            value: $this.val()
        }
    }).done(function(json) {
        console.log(json);
    }).fail(function(x, text, error) {
        // TODO unset
        console.log("Failed request");
        console.log(text);
        console.log(error);
    });
});
