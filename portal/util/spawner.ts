import { SpawnOptionsWithoutStdio, spawn } from "child_process";
import { createReadStream } from "fs";

export class Spawner {
    constructor(readonly executable: string, readonly args?: string[], readonly opts?: SpawnOptionsWithoutStdio) {}

    run(stdinFile?: string): Promise<string> {
        const proc = spawn(this.executable, this.args, this.opts);
        if (stdinFile) {
            const stream = createReadStream(stdinFile);
            stream.pipe(proc.stdin);
        }
        
        return new Promise((resolve, reject) => {
            const stdoutBuf: string[] = [];

            proc.stdout.on('data', (data) => {
                stdoutBuf.push(data);
            });
            proc.stderr.on('data', (data) => {
                console.log(`[${this.executable} STDERR] ${data}`);
            });
            proc.on('error', (err) => {
                console.error(`Process '${this.executable}' aborted`);
                reject(err);
            });
            proc.on('exit', (code, signal) => {
                if (code == 0) {
                    resolve(stdoutBuf.join(''));
                } else {
                    console.error(`Process '${this.executable}' exited abnormally (status: ${code}, signal: ${signal})`);
                    reject(stdoutBuf.join(''));
                }
            })
        });
    }
}