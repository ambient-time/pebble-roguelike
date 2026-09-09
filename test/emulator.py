"""Native rectangular face screenshots; isolated persistent profiles, no physical watches.

Run with the installed pebble-tool Python after pebble build.
"""
import argparse
import datetime
import hashlib
import json
import os
import re
from pathlib import Path
import time
import tempfile
import subprocess
from uuid import UUID
from types import SimpleNamespace

SDK=Path.home()/'Library/Application Support/Pebble SDK/SDKs/4.33.1'
toolchain=str(SDK.resolve()/'toolchain/bin')
os.environ.setdefault('PEBBLE_QEMU_PATH',toolchain+'/qemu-pebble')
os.environ['PATH']=toolchain+os.pathsep+os.environ['PATH']
import png
from pebble_tool.commands.screenshot import ScreenshotCommand
from pebble_tool.commands.install import ToolAppInstaller
from libpebble2.protocol.apps import AppRunState,AppRunStateStart
from libpebble2.protocol.logs import AppLogMessage,AppLogShippingControl
import pebble_tool.sdk.emulator as emulator
from libpebble2.communication.transports.qemu.protocol import QemuButton,QemuTap
from pebble_tool.commands.emucontrol import send_data_to_qemu

ROOT=Path(__file__).resolve().parent.parent
PBW=ROOT/'build'/f'{ROOT.name}.pbw' # The SDK names the bundle after its checkout folder.
APP=UUID(json.loads((ROOT/'package.json').read_text())['pebble']['uuid'])

def run(platform,fresh=False,record=0):
    installed_sha=hashlib.sha256(PBW.read_bytes()).hexdigest()
    out=ROOT/'build/evidence';out.mkdir(exist_ok=True)
    state=Path(tempfile.mkdtemp(prefix='emulator-fresh-',dir=ROOT/'build')) if fresh else ROOT/'build/emulator-state'
    state.mkdir(exist_ok=True)
    def persist(target,version=None):
        p=state/target;p.mkdir(exist_ok=True);return str(p)
    emulator.get_sdk_persist_dir=persist
    emulator.get_emulator_info_path=lambda:str(state/'emulators.json')
    emulator.get_default_account=lambda:SimpleNamespace(is_logged_in=False)
    bridge_log=(out/f'{platform}-bridge.log').open('w')
    emulator.ManagedEmulatorTransport._get_output=lambda self:bridge_log
    cmd=ScreenshotCommand();cmd._set_debugging(0)
    def shutdown():
        info=emulator.get_emulator_info(platform,'4.33.1')
        if info:
            for key in ('qemu','pypkjs','websockify'):
                pid=info.get(key,{}).get('pid')
                if not pid:continue
                process=subprocess.run(['ps','-p',str(pid),'-o','command='],capture_output=True,text=True)
                if process.returncode==0 and str(state/platform) not in process.stdout:
                    raise RuntimeError(f'Refusing to stop PID {pid}: it no longer belongs to this test profile.')
        cmd._shutdown_platform_emulator(platform,'4.33.1')
    # Only this test's private profile: clear a bridge left by interrupted runs.
    shutdown()
    try:
        watch=cmd._connect_emulator(platform,'4.33.1');cmd.pebble=watch
    except BaseException:
        shutdown();bridge_log.close()
        raise
    logs=[];frames=[]
    def log(packet):
        logs.append(str(packet.message));print(str(packet.message),flush=True)
    handle=watch.register_endpoint(AppLogMessage,log)
    watch.send_packet(AppLogShippingControl(enable=True))
    args=argparse.Namespace(no_correction=True,scale=1,no_open=True,v=0)
    def grab(name,clock=None):
        if clock:
            h,m,s=clock
            target=datetime.datetime.now().replace(hour=h,minute=m,second=s,microsecond=0)
            start=len(logs)
            cmd._set_time(watch,target);time.sleep(.35)
            cmd._set_time(watch,target)
            expected=f'time={h:02d}:{m:02d}:'
            deadline=time.monotonic()+5
            while not any(expected in line for line in logs[start:]) and time.monotonic()<deadline:
                time.sleep(.1)
            assert any(expected in line for line in logs[start:]),f'Watch did not render requested time {h}:{m}'
            time.sleep(.3)
        data=cmd._grab_processed_image(args,show_progress=False)
        file=out/f'{platform}-{name}.png';png.from_array(data,mode='RGBA;8').save(str(file))
        frames.append(str(file.relative_to(ROOT)))
        return data
    try:
        time.sleep(4)
        send_data_to_qemu(watch.transport,QemuButton(state=QemuButton.Button.Back))
        time.sleep(.2)
        send_data_to_qemu(watch.transport,QemuButton(state=0))
        time.sleep(.5)
        grab('boot')
        ToolAppInstaller(watch,str(PBW),quiet=True).install()
        watch.send_packet(AppRunState(data=AppRunStateStart(uuid=APP)))
        time.sleep(2)
        hero=grab('hero',(10,8,0));time.sleep(2.1)
        later=grab('later');assert hero!=later,'No second-driven movement'
        for name,clock in [('noon',(12,0,0)),('quarter',(3,15,45)),('evening',(18,30,30)),('floor-room',(17,29,15))]:
            grab(name,clock)
        for name,clock,expected in [('noon',(11,59,58),'time=12:00:'),('midnight',(23,59,58),'time=00:00:')]:
            grab('before-'+name,clock)
            start=len(logs);deadline=time.monotonic()+5
            while not any(expected in line for line in logs[start:]) and time.monotonic()<deadline:
                time.sleep(.1)
            assert any(expected in line for line in logs[start:]),f'No natural {name} rollover'
            grab(name+'-rollover')
        if record:
            grab('record-00',(11,24,30))
            start=time.monotonic()
            for i in range(1,record):
                time.sleep(max(0,start+i-time.monotonic()))
                grab(f'record-{i:02d}')
        assert any('time=' in line for line in logs),'No native app frame log'
        assert not any(any(term in line.lower() for term in ('crash','fault','allocation failed')) for line in logs),'Native fault logged'
        assert hashlib.sha256(PBW.read_bytes()).hexdigest()==installed_sha,'PBW changed during native verification'
        report={'platform':platform,'pbwSHA256':installed_sha,
                'capturedAt':datetime.datetime.now(datetime.timezone.utc).isoformat(),
                'secondsChanged':True,'naturalNoonRollover':True,'naturalMidnightRollover':True,
                'cadenceRequestedHz':1,'frames':frames,'logs':logs,
                'limits':'Isolated emulator. Frames require visual inspection; not physical-device power/readability evidence.'}
        (out/f'{platform}-report.json').write_text(json.dumps(report,indent=2)+'\n')
        print(json.dumps(report,indent=2),flush=True)
    finally:
        watch.unregister_endpoint(handle);cmd._close_pebble_connection(watch);cmd.pebble=None
        shutdown();bridge_log.close()

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('platform',choices=['emery','diorite','basalt','flint'])
    parser.add_argument('--fresh',action='store_true',help='Use a new isolated profile; preserve existing flash images.')
    parser.add_argument('--record',type=int,default=0,help='Also capture 0–60 one-second frames without changing the watch clock.')
    args=parser.parse_args()
    if not 0<=args.record<=60:parser.error('--record must be between 0 and 60')
    run(args.platform,args.fresh,args.record)
