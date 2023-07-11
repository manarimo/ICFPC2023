import useSWR from "swr";

export interface RawSolution {
  solverName: string;
  problemId: number;
  score: number;
  solutionPath: string;
  tag?: string;
}

const useSolutions = (tag?: string) => {
  const url = () => {
    let filename = 'solutions.json';
    if (tag) {
      filename = `solutions_${tag}.json`;
    }
    return `http://icfpc2023-manarimo-3mrzsd.s3-website-ap-northeast-1.amazonaws.com/snapshot/${filename}`;
  };
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const data = await response.json();
    return data as {
      solutions: RawSolution[];
    };
  };
  return useSWR(url, fetcher);
};
export default useSolutions;
