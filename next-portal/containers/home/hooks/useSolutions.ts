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
    let q = '';
    if (tag) {
      q = `?tag=${tag}`;
    }
    return `https://vwbqm1f1u5.execute-api.ap-northeast-1.amazonaws.com/prod/solutions${q}`
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
