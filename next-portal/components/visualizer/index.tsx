import { useEffect, useState } from "react";
import { render } from "./render";
import React from "react";

type Props = {
  problem: Parameters<typeof render>[1];
  solution: Parameters<typeof render>[2];
};

const Canvas = ({ problem, solution }: Props) => {
  const [context, setContext] = useState<CanvasRenderingContext2D | null>(null);
  useEffect(() => {
    const canvas = document.getElementById("canvas") as HTMLCanvasElement;
    const canvasContext = canvas.getContext("2d");
    setContext(canvasContext);
  }, [setContext]);

  useEffect(() => {
    if (context) {
      render(context, problem, solution);
    }
  }, [context, problem, solution]);

  return <canvas id="canvas" width="1000" height="1000" />;
};

export default Canvas;
