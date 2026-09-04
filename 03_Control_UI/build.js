const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

const target = process.argv[2] || 'simple'; // 'simple' or 'full'
const baseDir = __dirname;

console.log(`========================================`);
console.log(` H2 SCADA EXE Builder (${target.toUpperCase()} Version)`);
console.log(`========================================`);

if (target === 'simple') {
  console.log('[1/3] Setting Main View to H2ControlSimple.html...');
  fs.copyFileSync(
    path.join(baseDir, 'H2ControlSimple.html'),
    path.join(baseDir, 'index.html')
  );
} else {
  console.log('[1/3] Setting Main View to H2ControlTest.html...');
  fs.copyFileSync(
    path.join(baseDir, 'H2ControlTest.html'),
    path.join(baseDir, 'index.html')
  );
}

console.log('[2/3] Installing build dependencies...');
execSync('npm.cmd install', { cwd: baseDir, stdio: 'inherit' });

console.log('[3/3] Packaging Standalone Executables (.exe)...');
process.env.CSC_IDENTITY_AUTO_DISCOVERY = 'false';
process.env.WIN_CSC_LINK = '';
execSync('npm.cmd run dist', { cwd: baseDir, stdio: 'inherit' });

console.log('\n========================================');
console.log(' [SUCCESS] Standalone Executable (.exe) Built Successfully!');
console.log('========================================\n');

try {
  execSync('explorer dist', { cwd: baseDir });
} catch (e) {
  // ignore explorer launch error
}
