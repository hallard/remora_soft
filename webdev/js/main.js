/*
* Project: Remora = v1.3.3
* Description: IHM Web for Remora
* Author: Charles Henri Hallard, Thibaud, Manuel Hervo
* License: MIT License
* Website: https://github.com/hallard/remora_soft
*/

/* global define:false, require: false, jQuery:false */

+function ($) {
  var Timer_sys,
      Timer_tinfo,
      elapsed = 0,
      debug = true;

  function Notify(mydelay, myicon, mytype, mytitle, mymsg) {
    $('body').addClass('loaded');
    $.notify(
      { icon:'glyphicon glyphicon-'+myicon,
        title:'&nbsp;<strong>'+mytitle+'</strong>',
        message:'<p>'+mymsg+'</p>',
      },{
        type:mytype,
        //showProgressbar: true,
        animate:{enter:'animated fadeInDown',exit:'animated fadeOutUp',delay:mydelay*1000}
      }
    );
  }

  function progressUpload(data) {
    if(data.lengthComputable) {
      var pe = (data.loaded/data.total*100).toFixed(0) ;
      $('#pfw').css('width', pe +'%');
      $('#psfw').text(formatSize(data.loaded)+' / '+formatSize(data.total));
    }
  }

  function waitReboot() {
    var url = location.protocol+'//'+location.hostname+(location.port ? ':'+location.port: '') + '/#tab_sys' ;
    $('#txt_srv').text('Tentative de connexion à '+url);
    $('#mdl_wait').modal();

    var thistimer = setInterval(function() {
      $.ajax({
        cache: false,
        type: 'GET',
        url: '/hb.htm',
        timeout: 900,
        success: function(data, textStatus, XMLHttpRequest) {
        if (debug) console.log(data);
        if (data === 'OK') {
            $('#mdl_wait').modal('hide');
            clearInterval(thistimer);
            window.location = url;
            location.reload();
            elapsed=0;
        }
        }
      });
      elapsed++;
      $('#txt_elapsed').text('Temps écoulé ' + elapsed + ' s');
    }
    ,1000);
  }

  function addZoneTemplate(id, zones) {
    // console.log('addZoneTemplate ', id, zones);
    var template = '<div class="col-sm-6 col-md-4 zone">'
           + '  <div style="min-height:80px;" class="thumbnail" data-zone="#zone#" title="Gestion de la zone #zone#">'
           + '    <div class="caption"><h5>Zone #zone#</h5><span class="icon iconCmd" style="font-size: 3.5em;"></span>'
           + '    <div>'
           + '      <a href="#" class="actions btn btn-default conf" role="button">Confort</a>'
           + '      <a href="#" class="actions btn btn-default eco" role="button">ECO</a>'
           + '      <a href="#" class="actions btn btn-default hg" role="button">hors gel</a>'
           + '      <a href="#" class="actions btn btn-default off" role="button">arrêt</a>'
           + '    </div>'
           + '  </div>'
           + '</div>';
    template = template.replace(/#zone#/g, id.replace('fp', ''));
    // console.log('template: ' + template);
    // console.log('tab_fp .panel-body', $('#tab_fp .panel-body'));
    $('#tab_fp .zones').append(template); // On ajoute la template dans le body du panel
    var $div = $('#tab_fp .zones div.zone:last');
    // console.log('div.zone last', $div);
    activeZone(id.replace('fp', ''), zones[id]); // On active le bon bouton et l'image associée
    // On ajoute le bind sur click sur les boutons
    $('.actions', $div).on('click', function(e) {
      e.stopPropagation();
      e.preventDefault();
      var $this = $(this),
      zone = $this.parents('.thumbnail').data('zone');
      if ($this.hasClass('conf')) {
        sendOrdre(zone, 'C');
      } else if ($this.hasClass('eco')) {
        sendOrdre(zone, 'E');
      } else if ($this.hasClass('hg')) {
        sendOrdre(zone, 'H');
      } else if ($this.hasClass('off')) {
        sendOrdre(zone, 'A');
      }
    });
  }

  function sendOrdre(id, ordre) {
    $('body').removeClass('loaded');
    if (debug) console.log('sendOrdre ', id, ordre);
    var request = {setfp: id + ordre};
    if (id == false) {
      request = {fp: Array(8).join(ordre)};
    }
    if (debug) console.log('request', request);
    $.getJSON('/', request)
      .done(function(data, textStatus, jqXHR) {
        if (debug) console.log('response ', request, data, textStatus, jqXHR);
        if (data.hasOwnProperty('response') && data.response == 0) {
          if (id === false) {
            $('.zones .zone').each(function(index, elt) {
              var id = $('.thumbnail', elt).data('zone');
              activeZone(id, ordre);
            });
          } else {
            activeZone(id, ordre);
          }
        } else {
          if (debug) console.error('Error lors de l\'envoi d\'ordre(%s) pour fil pilote(%s):', ordre, id, data);
          Notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour fil pilote #'+id, jqXHR.status+' '+data);
        }
        $('body').addClass('loaded');
      }).fail(function( jqXHR, textStatus, error ) {
        var err = textStatus + ", " + error;
        if (debug) console.error("Request Failed: " + err);
        Notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour fil pilote #'+id, jqXHR.status+' '+err);
        // console.log(jqxhr);
      });
  }

  function activeZone(id, state) {
    if (debug) console.log('activeZone', id, state);
    
    var $div = $('div.thumbnail[data-zone="'+id+'"]'),
        $icon = $('span.icon', $div),
        active,
        img;
    
    switch (state) {
      case 'C':
        active = 'a.conf';
        img = 'jeedom-pilote-conf';
      break;
      case 'E':
        active = 'a.eco';
        img = 'jeedom-pilote-eco';
      break;
      case 'H':
        active = 'a.hg';
        img = 'jeedom-pilote-hg';
      break;
      case 'A':
        active = 'a.off';
        img = 'jeedom-pilote-off';
      break;
    }
    if (debug) console.log('active: %s - img: %s', active, img);
    $icon.empty();
    $('a.actions', $div).removeClass('active btn-success');
    $(active, $div).addClass('active btn-success');
    $icon.append('<i class="icon ' + img + '"></i>');
  }

  function sendRelais(api, ordre) {
    $('body').removeClass('loaded');
    if (debug) console.log('sendRelais[%s] ', api, ordre);
    var request = {};
    request[api] = ordre;
    if (debug) console.log('request', request);
    $.getJSON('/', request)
      .done(function(data, textStatus, jqXHR) {
        if (debug) console.log('response ', request, data, textStatus, jqXHR);
        if (data.hasOwnProperty('response') && data.response == 0) {
          $.getJSON('/relais', function(data) {
            if (data.hasOwnProperty('relais') && data.hasOwnProperty('fnct_relais')) {
              activeRelais(data.fnct_relais, data.relais);
            }
          });
        } else {
          if (debug) console.error('Error lors de l\'envoi d\'ordre(%s) pour relais:', ordre, data);
          Notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour relais', jqXHR.status+' '+data);
        }
        $('body').addClass('loaded');
      }).fail(function( jqXHR, textStatus, error ) {
        var err = textStatus + ", " + error;
        if (debug) console.error("Request Failed: " + err);
        Notify(4, 'remove', 'danger', 'Error ordre '+ordre+' pour relais', jqXHR.status+' '+err);
        // console.log(jqxhr);
      });
  }

  function activeRelais(fnct, state) {
    if (debug) console.log('activeZone', fnct, state);
    var $icon = $('#tab_fp .relais span.icon'),
        active = '#tab_fp .relais',
        img = 'jeedom-relais-on';

    if (parseInt(state, 10) == 0) {
      img = 'jeedom-relais-off';
    }
    switch (parseInt(fnct)) {
      case 0:
        active += ' a.off';
        break;
      case 1:
        active += ' a.force';
        break;
      case 2:
      default:
        active += ' a.auto';
        break;
    }
    if (debug) console.log('active: %s - img: %s', active, img);
    $icon.empty();
    $icon.data('val', state);
    $('#tab_fp .relais a.actions').removeClass('active btn-success');
    $(active).addClass('active btn-success');
    $icon.append('<i class="icon ' + img + '"></i>');
  }

  $(function() {

    $('a[data-toggle="tab"]').on('shown.bs.tab', function (e) {
      clearTimeout(Timer_sys);
      clearTimeout(Timer_tinfo);
      var target = $(e.target).attr("href")
      if (debug) console.log('activated ' + target);

      // IE10, Firefox, Chrome, etc.
      if (history.pushState)
        window.history.pushState(null, null, target);
      else
        window.location.hash = target;

      if (target == '#tab_tinfo') {
        $('#tab_tinfo_data').bootstrapTable('refresh',{silent:true, url:'/tinfo.json'});
      } else if (target == '#tab_sys') {
        $('#tab_sys_data').bootstrapTable('refresh',{silent:true, url:'/system.json'});
      } else if (target == '#tab_fs') {
        $.getJSON( "/spiffs.json", function(spiffs_data) {
          var pb, pe, cl;
          total= spiffs_data.spiffs[0].Total;
          used = spiffs_data.spiffs[0].Used;
          freeram = spiffs_data.spiffs[0].Ram;

          $('#tab_fs_data').bootstrapTable('load', spiffs_data.files, {silent:true, showLoading:true});

          pe = parseInt(used*100/total);
          if (isNaN(pe))
            pe = 0;
          cl = 'success';
          if (pe > 70) cl = 'warning';
          if (pe > 85) cl = 'danger';

          cl = 'progress-bar-' + cl;
          if (pe > 0) {
            $('#sspiffs').text(pe+'%');
          }
          $('#fs_use').text(formatSize(used)+' / '+formatSize(total));
          $('#pfsbar').attr('class','progress-bar '+cl);
          $('#pfsbar').css('width', pe+'%');

        })
        .fail(function() { console.error( "error while requestiong spiffs data" );  })
      } else if (target == '#tab_cfg') {
        $.getJSON( "/config.json", function(form_data) {
          $("#frm_config").autofill(form_data);
          })
          .fail(function() { console.error( "error while requestiong configuration data" ); })

        $('#tab_scan_data').bootstrapTable('refresh',{silent:true, showLoading:true, url:'/wifiscan.json'});
      }
      // Onglet de gestion des zones
      else if (target == '#tab_fp') {
        $('body').removeClass('loaded'); // On affiche le loader
        // On récupère l'état de toutes les zones
        $.getJSON('/fp', function(data) {
          $('#tab_fp .zones').empty(); // On vide l'espace d'affichage des zones
          for (var k in data) {
            addZoneTemplate(k, data); // On ajoute l'affichage d'une zone
          }
          $('body').addClass('loaded'); // On masque le loader
          // On ajoute un bind sur les boutons d'action généraux
          $('#tab_fp .all .actions').unbind('click').click(function(e) {
            e.stopPropagation();
            e.preventDefault();
            var $this = $(this);
            if ($this.hasClass('conf')) {
              sendOrdre(false, 'C');
            } else if ($this.hasClass('eco')) {
              sendOrdre(false, 'E');
            } else if ($this.hasClass('hg')) {
              sendOrdre(false, 'H');
            } else if ($this.hasClass('off')) {
              sendOrdre(false, 'A');
            }
          });
        });
        $.getJSON('/relais', function(data) {
          if (data.hasOwnProperty('fnct_relais')) {
            activeRelais(data.fnct_relais, data.relais);
          }
          $('#tab_fp .relais .iconCmd').unbind('click').click(function(e) {
            e.stopPropagation();
            e.preventDefault();
            var $this = $(this);
            if ($this.data('val') == 0) {
              sendRelais('relais', 1);
            } else {
              sendRelais('relais', 0);
            }
          })
          $('#tab_fp .relais .actions').unbind('click').click(function(e) {
            e.stopPropagation();
            e.preventDefault();
            var $this = $(this);
            if ($this.hasClass('force')) {
              sendRelais('frelais', 1);
            } else if ($this.hasClass('auto')) {
              sendRelais('frelais', 2);
            } else if ($this.hasClass('off')) {
              sendRelais('frelais', 0);
            }
          });
        });
      }
    });

    $('#tab_tinfo_data')
      .on('load-success.bs.table', function (e, data) {
        if (debug) console.log('#tab_tinfo_data loaded', e, data);
        if ($('.nav-tabs .active > a').attr('href')=='#tab_tinfo') {
          Timer_tinfo=setTimeout(function(){$('#tab_tinfo_data').bootstrapTable('refresh',{silent: true})},1000);
        }
      }).on('load-error.bs.table', function (e, status, res) {
        if (debug) console.log('Event: load-error.bs.table on tab_tinfo_data', e, status, res);
        if (status === 404 && res.hasOwnProperty('responseJSON') && res.responseJSON.hasOwnProperty('result')) {
          $('#tab_tinfo_data .no-records-found td').html("Télé-information désactivée");
        }
      });
    $('#tab_sys_data').on('load-success.bs.table', function (e, data) {
      if (debug) console.log('#tab_sys_data loaded');
      if ($('.nav-tabs .active > a').attr('href')=='#tab_sys')
      Timer_sys=setTimeout(function(){$('#tab_sys_data').bootstrapTable('refresh',{silent: true})},1000);
    })
    $('#tab_fs_data')
      .on('load-success.bs.table', function (e, data) {
        if (debug) console.log('#tab_fs_data loaded');
      })
      .on('load-error.bs.table', function (e, status, res) {
        if (debug) console.log('Event: load-error.bs.table on tab_fs_data', e, status, res);
          // myTimer=setInterval(function(){myRefresh()},5000);
      });
    $('#tab_scan_data').on('load-success.bs.table', function (e, data) {
      if (debug) console.log('#tab_scan_data data loaded', data);
      //$(this).hide();
      if (data.status == 'OK') {
        $('#tab_scan_data').bootstrapTable('load', data.result);
      }
    });
    $('#tab_scan').on('click-row.bs.table', function (e, name, args) {
      var $form = $('#tab_cfg');
      $('#ssid').val(name.ssid);
      setTimeout(function(){$('#psk').focus()},500);
      $('#tab_scan').modal('hide');
    });
    $('#btn_scan').click(function () {
      $('#tab_scan_data').bootstrapTable('refresh',{url:'/wifiscan.json',showLoading:false,silent:true});
    });
    $('#btn_reset').click(function () {
      $.post('/factory_reset');
      waitReboot();
      return false;
    });
    $('#btn_reboot').click(function () {
      $.post('/reset');
      waitReboot();
      return false;
    });

    $(document)
      .on('change', '.btn-file :file', function() {
        var input = $(this),
            numFiles = input.get(0).files ? input.get(0).files.length : 1,
            label = input.val().replace(/\\/g, '/').replace(/.*\//, '');

        input.trigger('fileselect', [numFiles, label]);
      })
      .on('show.bs.collapse', '.panel-collapse', function () {
        var $span = $(this).parents('.panel').find('span.pull-right.glyphicon');
        $span.removeClass('glyphicon-chevron-down').addClass('glyphicon-chevron-up');
      })
      .on('hide.bs.collapse', '.panel-collapse', function () {
        var $span = $(this).parents('.panel').find('span.pull-right.glyphicon');
        $span.removeClass('glyphicon-chevron-up').addClass('glyphicon-chevron-down');
      });

    $('#frm_config').validator().on('submit', function (e) {
      // everything looks good!
      if (!e.isDefaultPrevented()) {
        e.preventDefault();
        if (debug) console.log("Form Submit");

        $.post('/config_form.json', $("#frm_config").serialize())
          .done( function(msg, textStatus, xhr) {
            Notify(2, 'ok', 'success', 'Enregistrement effectué', xhr.status+' '+msg);
          })
          .fail( function(xhr, textStatus, errorThrown) {
            Notify(4, 'remove', 'danger', 'Erreur lors de l\'enregistrement', xhr.status+' '+errorThrown);
          }
      );
      }
    });

    $('#file_fw').change(function() {
      var $txt = $('#txt_upload_fw'),
          $btn = $('#btn_upload_fw'),
          ok = true,
          f = this.files[0],
          name = f.name.toLowerCase(),
          size = f.size,
          type = f.type,
          html = 'Fichier:' + name + '&nbsp;&nbsp;type:' + type + '&nbsp;&nbsp;taille:' + size + ' octets'
      if (debug) console.log('name: ' + name);
      if (debug) console.log('size: ' + size);
      if (debug) console.log('type: ' + type);

      $('#pgfw').removeClass('show').addClass('hide');
      $('#sfw').text(name + ' : ');

      if (!f.type.match('application/octet-stream')) {
        Notify(3, 'remove', 'danger', 'Type de fichier non conforme', 'Le fichier de mise à jour doit être un fichier binaire');
        ok = false;
      //} else if (name!="remora_soft.cpp.bin" && name!="remora_soft.spiffs.bin") {
      } else if (! /^remora_soft.*.bin$/i.test(name) ) {
        Notify(5, 'remove', 'danger', 'Nom de fichier incorrect', 'Le fichier de mise à jour doit être nommé <ul><li>remora_soft.*.bin (Micro-logiciel) ou</li><li>remora_soft.spiffs.bin (Système de fichiers)</li></ul>');
        ok = false;
      }
      if (ok) {
        $btn.removeClass('hide');
        if (name === "remora_soft.spiffs.bin") {
          label = 'Mise à jour SPIFFS';
        } else {
          label = 'Mise à jour Micro-Logiciel';
        }
        $btn.val(label);
        $('#fw_info').html('<strong>' + label + '</strong> ' + html);
      } else {
        $txt.attr('readonly', '');
        $txt.val('');
        $txt.attr('readonly', 'readonly');
        $btn.addClass('hide');
      }
      return ok;
    });

    $('#btn_upload_fw').click(function() {
      var formData = new FormData($('#frm_fw')[0]);
      $.ajax({
        url: '/update',
        type: 'POST',
        data: formData,
        cache: false,
        contentType: false,
        processData: false,
        xhr: function() {
          var myXhr = $.ajaxSettings.xhr();
          if(myXhr.upload)
            myXhr.upload.addEventListener('progress', progressUpload, false);
          return myXhr;
        },
        beforeSend: function () {
          $('#pgfw').removeClass('hide');
        },
        success: function(msg, textStatus, xhr) {
          Notify(2, 'floppy-saved', 'success','Envoi de la mise à jour terminé', '<strong>'+xhr.status+'</strong> '+msg);
          waitReboot();
        },
        error: function(xhr, textStatus, errorThrown) {
          $('#pfw').removeClass('progress-bar-success').addClass('progress-bar-danger');
          Notify(4, 'floppy-remove', 'danger', 'Erreur lors de la mise à jour du fichier '+name,'<strong>'+xhr.status+'</strong> '+errorThrown);
        }
      });
    });

    $('#btn_test').click(function(){ waitReboot(); });

    var url = document.location.toString(),
        full = location.protocol+'//'+location.hostname+(location.port ? ':'+location.port: '');
    if (debug) console.log ('url: ' + url);
    if (debug) console.log ('full: ' + full);
    if (debug) console.log ('port: ' + location.port);

    var tab = 'tab_sys';

    $('.nav-tabs a').on('shown', function(e) {
      window.location.hash = e.target.hash;
    });
    if (url.match('#')) {
      tab = url.split('#')[1];
    }
    $('.nav-tabs a[href=#' + tab + ']').tab('show').trigger('shown');

    // enlever le loader, tout est prêt
    $('body').addClass('loaded');
  });
}(jQuery);