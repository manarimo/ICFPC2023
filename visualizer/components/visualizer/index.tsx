import { useEffect, useState } from "react";
import { render } from "./render";

type Props = {
  problem: Parameters<typeof render>[1];
};

const Canvas = ({ problem }: Props) => {
  const [context, setContext] = useState<CanvasRenderingContext2D | null>(null);
  useEffect(() => {
    const canvas = document.getElementById("canvas") as HTMLCanvasElement;
    const canvasContext = canvas.getContext("2d");
    setContext(canvasContext);
  }, [setContext]);

  useEffect(() => {
    if (context) {
      render(context, problem);
    }
  }, [context, problem]);

  return <canvas width="1500" height="750" id="canvas"></canvas>;
};

export default Canvas;
