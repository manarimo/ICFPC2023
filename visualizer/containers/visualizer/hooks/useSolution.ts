import useSWR from "swr";

const useSolution = (solver: string | undefined, problemId: number) => {
  const url = solver ? `/solutions/${solver}/${problemId}.json` : null;
  const fetcher = async (url: string) => {
    const response = await fetch(url);
    const body = await response.json();
    return body as {
      placements: {
        x: number;
        y: number;
      }[];
    };
  };
  return useSWR(url, fetcher);
};

export default useSolution;
