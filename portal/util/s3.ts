import { GetObjectCommand, PutObjectCommand, PutObjectCommandInput, S3 } from "@aws-sdk/client-s3";
import { createReadStream } from "fs";
import { writeFile } from "fs/promises";

export class S3Util {
    private _s3: S3;

    constructor() {
        this._s3 = new S3({});
    }

    async downloadS3Object(s3Path: string, fsPath: string): Promise<void> {
        const command = new GetObjectCommand({
            Bucket: "icfpc2023-manarimo-3mrzsd",
            Key: s3Path
        });
        const result = await this._s3.send(command);
        const content = await result.Body?.transformToByteArray();
        if (content == undefined) {
            throw new Error(`Failed to download ${s3Path}`);
        }
        await writeFile(fsPath, content);
    }

    async uploadS3Object(s3Path: string, content: string): Promise<void> {
        const command = new PutObjectCommand({
            Bucket: "icfpc2023-manarimo-3mrzsd",
            Key: s3Path,
            Body: content
        });
        await this._s3.send(command);
    }

    async uploadS3ObjectFromFile(s3Path: string, file: string, contentType?: string): Promise<void> {
        const buf = createReadStream(file);
        const input: PutObjectCommandInput = {
            Bucket: "icfpc2023-manarimo-3mrzsd",
            Key: s3Path,
            Body: buf,
        };
        if (contentType != undefined) {
            input.ContentType = contentType;
        }

        const command = new PutObjectCommand(input);
        await this._s3.send(command);
        buf.close();
    }
}