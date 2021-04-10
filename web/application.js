$(function() {
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

    // Check for software upgrade
    var swv = $('#swVersion')
    if(meters.length > 0 && swv.length == 1 && swv.text() != "SNAPSHOT") {
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
