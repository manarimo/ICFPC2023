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
      context.canvas.height = window.innerHeight - 10;
      context.canvas.width = window.innerWidth - 10;
      render(context, problem, solution);
    }
  }, [context, problem, solution]);

  return <canvas id="canvas" />;
};

export default Canvas;
