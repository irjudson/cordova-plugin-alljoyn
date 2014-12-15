#!/usr/bin/env node

var path = require('path');
var fs = require('fs');
var exec = require('child_process').exec;

var ajtclDirectory = path.join('plugins', 'org.allseen.alljoyn', 'src', 'ajtcl');
var ajtclUpstream = 'https://git.allseenalliance.org/gerrit/core/ajtcl';
var ajtclBranch = 'RB14.06';

if (fs.existsSync(ajtclDirectory)) {
  console.log("Found ajtcl from: " + path.resolve(ajtclDirectory));
} else {
  console.log('Cloning ajtcl from: ' + ajtclUpstream);
  exec('git clone  ' + ajtclUpstream + ' ' + ajtclDirectory,
    function (error, stdout, stderr) {
      if (error !== null) {
        console.log('Git clone failed to error: ' + error);
      } else {
      console.log('Checking out branch: ' + ajtclBranch);
      exec('git -C ' + ajtclDirectory + ' checkout ' + ajtclBranch);
    }
  });
}