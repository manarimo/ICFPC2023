import useSWR from "swr";

const useProblem = (problemId: number) => {
  const url = `/problems/${problemId}.json`;
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const body = await response.json();
    return {
      room: {
        width: body["room_width"] as number,
        height: body["room_height"] as number,
      },
      stage: {
        width: body["stage_width"] as number,
        height: body["stage_height"] as number,
        left: body["stage_bottom_left"][0] as number,
        bottom: body["stage_bottom_left"][1] as number,
      },
      musicians: body["musicians"] as number[],
      attendees: body["attendees"] as {
        x: number;
        y: number;
        tastes: number[];
      }[],
    };
  };
  return useSWR(url, fetcher);
};

export default useProblem;
